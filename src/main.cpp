#include "common.hpp"
#include "window.hpp"
#include "image.hpp"
#include "scene.hpp"
#include "renderer.hpp"

MUU_DISABLE_WARNINGS;
#include <memory>
#include <filesystem>
#include <SDL.h>
#include <iostream>
#include <numeric>
#include <exception>
#include <SDL_main.h>
#include <atomic>
#include <span>
#include <muu/thread_pool.h>
#include <muu/strings.h>
#include <argparse/argparse.hpp>
#include <imgui.h>
MUU_ENABLE_WARNINGS;

using namespace rt;
namespace fs = std::filesystem;

namespace
{
	static std::atomic_bool should_quit = false;

	template <typename Char, typename... Args>
	static void print(std::basic_ostream<Char>& os, Args&&... args)
	{
		(os << ... << static_cast<Args&&>(args));
		os << "\n"sv;
	}

	template <typename... Args>
	static void log(Args&&... args)
	{
		print(std::cout, static_cast<Args&&>(args)...);
	}

	template <typename... Args>
	static void error(Args&&... args)
	{
		print(std::cerr, "error: ", static_cast<Args&&>(args)...);
	}

	struct renderer
	{
		const renderers::description* description;
		std::unique_ptr<renderer_interface> object;

		MUU_PURE_INLINE_GETTER
		explicit operator bool() const noexcept
		{
			return !!object;
		}

		MUU_PURE_INLINE_GETTER
		renderer_interface* operator->() noexcept
		{
			return object.get();
		}
	};

	MUU_NODISCARD
	static const renderers::description* find_renderer_by_name_fuzzy(std::string_view name) noexcept
	{
		if (name.empty())
			return {};

		if (auto desc = renderers::find_by_name(name))
			return desc;

		for (auto& r : renderers::all())
			if (r.name.starts_with(name))
				return &r;

		return {};
	}

	static void run(const argparse::ArgumentParser& args)
	{
		bool renderer_changed	   = false;
		const auto create_renderer = [&](std::string_view name) -> renderer
		{
			auto desc = find_renderer_by_name_fuzzy(name);
			if (!desc)
			{
				error("no known renderer with name '", name, "'");
				return {};
			}
			assert(desc->name == name);
			assert(desc->create);

			renderer r;
			r.description = desc;
			r.object.reset(desc->create());
			log("created renderer: ", name);
			renderer_changed = true;
			return r;
		};

		renderer regular_renderer = create_renderer(muu::trim(args.get<std::string>("renderer")));
		renderer low_res_renderer = create_renderer("rasterizer"sv);
		rt::scene scene;
		time_point last_scene_write_check{};
		fs::file_time_type last_scene_write{};
		const auto reload_scene = [&](bool preserve_camera = true)
		{
			const auto prev_camera = scene.camera;
			const auto do_cam	   = muu::scope_guard{ [&]() noexcept
												   {
													  if (preserve_camera)
														  scene.camera = prev_camera;
												  } };

			try
			{
				const auto path = muu::trim(args.get<std::string>("scene"));
				if (path.empty())
					scene = scene::load_first_available();
				else
					scene = scene::load(path);
			}
			catch (const std::exception& ex)
			{
				error(ex.what());
				last_scene_write = {};
				return false;
			}

			last_scene_write_check = clock::now();
			if (!scene.path.empty())
			{
				try
				{
					last_scene_write = fs::last_write_time(scene.path);
				}
				catch (...)
				{
					last_scene_write = {};
				}
				log("scene '"sv, scene.path, "' loaded."sv);
			}
			else
				log("scene loaded."sv);

			return true;
		};

		auto win				= window{ "rt"s, { 800, 600 } };
		const auto update_title = [&]()
		{
			std::ostringstream ss;
			ss << "rt"sv;
			if (!scene.path.empty())
				ss << " - "sv << scene.path;
			if (regular_renderer)
				ss << " - "sv << regular_renderer.description->name;
			win.title(ss.str());
		};

		muu::thread_pool threads;

		bool reload_requested = true;
		bool first_loaded	  = false;
		bool mouse_dragging	  = false;
		float last_mouse_x	  = 0;
		float last_mouse_y	  = 0;
		float pitch_delta	  = 0.0f;
		float yaw_delta		  = 0.0f;

		time_point last_move_time = clock::now() - 1s;

		win.loop({ .key_down =
					   [&](int key) noexcept
				   {
					   log("key down: "sv, key);
					   if (win.key('+', '-', 61, 45))
					   {
						   const auto all_renderers = renderers::all();
						   auto renderer_index =
							   static_cast<size_t>(regular_renderer.description - renderers::all().data());
						   renderer_index =
							   static_cast<size_t>((static_cast<int>(renderer_index) + (win.key('+', 61) ? 1 : -1)))
							   % all_renderers.size();
						   log("renderer index", renderer_index);
						   regular_renderer = create_renderer(all_renderers[renderer_index].name);
					   }

					   if (win.key(27)) // esc key
						   should_quit = true;
					   if (key == 'r')
						   reload_requested = true;
				   },

				   .mouse_button_up =
					   [&](int key) noexcept
				   {
					   if (key == SDL_BUTTON_RIGHT)
					   {
						   mouse_dragging = false;
					   }
				   },

				   .mouse_button_down =
					   [&](int key, float x, float y) noexcept
				   {
					   if (key == SDL_BUTTON_RIGHT)
					   {
						   mouse_dragging = true;
						   last_mouse_x	  = x;
						   last_mouse_y	  = y;
					   }
				   },
				   .mouse_button_motion =
					   [&](float x, float y) noexcept
				   {
					   if (mouse_dragging)
					   {
						   float dx = x - last_mouse_x;
						   float dy = y - last_mouse_y;

						   yaw_delta += dx * 0.1f;
						   pitch_delta += dy * 0.1f;

						   last_mouse_x = x;
						   last_mouse_y = y;
					   }
				   },
				   .update = [&](float delta_time, bool& backbuffer_dirty) noexcept -> bool
				   {
					   if (first_loaded											//
						   && !scene.path.empty()								//
						   && last_scene_write_check.time_since_epoch().count() //
						   && last_scene_write.time_since_epoch().count()		//
						   && clock::now() - last_scene_write_check >= 0.5s)
					   {
						   last_scene_write_check = clock::now();
						   try
						   {
							   if (fs::last_write_time(scene.path) > last_scene_write)
								   reload_requested = true;
						   }
						   catch (...)
						   {}
					   }

					   ImGui::Begin("foo");
					   if (ImGui::Button("reload?"))
						   reload_requested = true;
					   ImGui::End();

					   bool reloaded_this_frame = false;
					   if (reload_requested)
					   {
						   reload_requested	   = false;
						   reloaded_this_frame = reload_scene(first_loaded);
						   first_loaded		   = first_loaded || reloaded_this_frame;
						   update_title();
					   }

					   bool moved_this_frame = false;
					   vec3 move_dir{};
					   if (mouse_dragging)
					   {
						   if (yaw_delta != 0.0f)
						   {
							   scene.camera.rotate_yaw(yaw_delta * delta_time);
							   yaw_delta		= 0.0f;
							   moved_this_frame = true;
						   }
						   if (pitch_delta != 0.0f)
						   {
							   scene.camera.rotate_pitch(pitch_delta * delta_time);
							   pitch_delta		= 0.0f;
							   moved_this_frame = true;
						   }
					   }
					   if (win.key('w', 1073741906))
						   move_dir += vec3::constants::forward;
					   if (win.key('a', 1073741904))
						   move_dir += vec3::constants::left;
					   if (win.key('s', 1073741905))
						   move_dir += vec3::constants::backward;
					   if (win.key('d', 1073741903))
						   move_dir += vec3::constants::right;
					   if (win.key(' '))
						   move_dir += vec3::constants::up;
					   if (win.key(1073742048))
						   move_dir += vec3::constants::down;
					   if (!muu::approx_zero(move_dir))
					   {
						   auto move = vec3::normalize(move_dir) * delta_time;
						   if (!muu::approx_zero(move))
						   {
							   scene.camera.pose(scene.camera.position() + move, scene.camera.rotation());
							   moved_this_frame = true;
						   }
					   }
					   if (mouse_dragging)
						   moved_this_frame = true;
					   if (moved_this_frame)
						   last_move_time = clock::now();
					   const auto prev_low_res = win.low_res;
					   win.low_res			   = (clock::now() - last_move_time) < 0.5s;
					   backbuffer_dirty		   = backbuffer_dirty || moved_this_frame || reloaded_this_frame
									   || renderer_changed || (win.low_res != prev_low_res);
					   renderer_changed = false;
					   return !should_quit;
				   },

				   .render =
					   [&](image_view pixels) noexcept
				   {
					   pixels.clear(colours::black);
					   if (auto& r = (win.low_res ? low_res_renderer : regular_renderer))
						   r->render(scene, pixels, threads);
				   }

		});
	}
}

int main(int argc, char** argv)
{
	try
	{
		argparse::ArgumentParser args{ "rt" };

		args.add_description("Renders a scene with a software renderer of your choosing.");

		args.add_argument("-l", "--list")
			.help("lists available renderers and exits") //
			.flag();

		args.add_argument("-s", "--scene")
			.help("scene TOML file") //
			.nargs(1u)
			.required()
			.default_value(""s)
			.metavar("<path>");

		args.add_argument("-r", "--renderer")
			.help("renderer name") //
			.nargs(1u)
			.required()
			.default_value(std::string{ find_renderer_by_name_fuzzy("mg")->name })
			.metavar("<name>");

		args.parse_args(argc, argv);

		if (args.get<bool>("list"))
		{
			for (auto& r : renderers::all())
				log(r.name);
			return 0;
		}

		log("working directory: "sv, fs::current_path().string());

		log("available renderers: "sv);
		for (auto& r : renderers::all())
			log("    ", r.name);

		run(args);
	}
	catch (const std::exception& ex)
	{
		error(ex.what());
		return 1;
	}
	catch (...)
	{
		error("An unspecified internal error occurred."sv);
		return 1;
	}

	return 0;
}
