#include "common.hpp"
#include "window.hpp"
#include "image.hpp"
#include "scene.hpp"
#include "renderer.hpp"
MUU_DISABLE_WARNINGS;
#include <memory>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <exception>
#include <SDL_main.h>
#include <atomic>
#include <span>
#include <muu/thread_pool.h>
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

	static void run(std::span<const char*> args)
	{
		log("working directory: "sv, fs::current_path().string());

		log("available renderers: "sv);
		for (auto& r : renderers::all())
			log("    ", r.name);

		const auto create_renderer = [](std::string_view name) -> renderer
		{
			auto desc = renderers::find_by_name(name);
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
			return r;
		};

		renderer regular_renderer = create_renderer("mg_scalar_ray_tracer"sv);
		renderer low_res_renderer = create_renderer("simple_rasterizer"sv);

		rt::scene scene;
		const auto reload = [&]()
		{
			try
			{
				scene = scene::load(args.empty() ? "" : args[0]);
				if (!scene.path.empty())
					log("scene '"sv, scene.path, "' loaded."sv);
				else
					log("scene loaded."sv);
				return true;
			}
			catch (const std::exception& ex)
			{
				error(ex.what());
				return false;
			}
		};
		reload();

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
		update_title();

		muu::thread_pool threads;
		bool reloaded_this_frame  = true;
		time_point last_move_time = clock::now() - 1s;
		win.loop({
			.key_down =
				[&](int key) noexcept
			{
				log("key down: "sv, key);

				if (key == ' ' && reload())
				{
					update_title();
					reloaded_this_frame = true;
				}
			},

			.update = [&](float delta_time, bool& backbuffer_dirty) noexcept -> bool
			{
				bool moved_this_frame = false;
				vec3 move_dir{};
				if (win.key('w', 1073741906))
					move_dir += vec3::constants::forward;
				if (win.key('a', 1073741904))
					move_dir += vec3::constants::left;
				if (win.key('s', 1073741905))
					move_dir += vec3::constants::backward;
				if (win.key('d', 1073741903))
					move_dir += vec3::constants::right;
				if (!muu::approx_zero(move_dir))
				{
					auto move = vec3::normalize(move_dir) * delta_time;
					if (!muu::approx_zero(move))
					{
						scene.camera.pose(scene.camera.position() + move, scene.camera.rotation());
						moved_this_frame = true;
						last_move_time	 = clock::now();
					}
				}

				const auto prev_low_res = win.low_res;
				win.low_res				= (clock::now() - last_move_time) < 0.5s;
				backbuffer_dirty		= moved_this_frame || reloaded_this_frame || (win.low_res != prev_low_res);
				reloaded_this_frame		= false;
				return !should_quit;
			},

			.render =
				[&](image_view pixels) noexcept
			{
				auto& r = (win.low_res ? low_res_renderer : regular_renderer);
				if (r)
					r->render(scene, pixels, threads);
			} //
		});
	}
}

int main(int argc, char** argv)
{
	try
	{
		run(std::span<const char*>{ static_cast<const char**>(static_cast<void*>(argv + 1)),
									static_cast<size_t>(argc - 1) });
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
