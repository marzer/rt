#include "common.hpp"
#include "window.hpp"
#include "image.hpp"
#include "scene.hpp"
#include "scalar_ray_tracer.hpp"
#include "simd_ray_tracer.hpp"
MUU_DISABLE_WARNINGS;
#include <memory>
#include <iostream>
#include <exception>
#include <SDL_main.h>
#include <atomic>
#include <span>
#include <muu/thread_pool.h>
MUU_ENABLE_WARNINGS;

using namespace rt;

namespace
{
	static std::atomic_bool should_quit = false;

	static void run(std::span<const char*> args)
	{
		rt::scene scene;
		const auto reload = [&]()
		{
			try
			{
				scene = scene::load(args.empty() ? "" : args[0]);
				return true;
			}
			catch (const std::exception& ex)
			{
				std::cerr << "FATAL ERROR: " << ex.what() << "\n";
				window::error_message_box("FATAL ERROR", ex.what());
				return false;
			}
		};
		reload();

		muu::thread_pool threads;

		std::unique_ptr<ray_tracer_interface> ray_tracer{ new scalar_ray_tracer };

		bool dirty					 = true;
		auto win					 = window{ "rt", { 800, 600 } };
		vec3 move_dir				 = {};
		time_point low_res_mode_time = clock::now();
		win.loop({ .key_down =
					   [&](int key) noexcept
				   {
					   switch (key)
					   {
						   case ' ':
							   if (reload())
							   {
								   scene.low_res_mode = win.low_res_mode;
								   dirty			  = true;
							   }
							   break;

						   case 97: move_dir.x += 1; break;
						   case 100: move_dir.x += -1; break;
						   case 119: move_dir.z += -1; break;
						   case 115: move_dir.z += 1; break;
					   }
				   },

				   .key_up =
					   [&](int key) noexcept
				   {
					   switch (key)
					   {
						   case 97: move_dir.x -= 1; break;
						   case 100: move_dir.x -= -1; break;
						   case 119: move_dir.z -= -1; break;
						   case 115: move_dir.z -= 1; break;
					   }
				   },

				   .update = [&](float delta_time, bool& backbuffer_dirty) noexcept -> bool //
				   {																		//
					   if (!muu::approx_zero(move_dir))
					   {
						   auto move = vec3::normalize(move_dir) * delta_time;
						   if (!muu::approx_zero(move))
						   {
							   scene.camera.pose(scene.camera.position() + move, scene.camera.rotation());
							   dirty			  = true;
							   low_res_mode_time  = clock::now();
							   win.low_res_mode	  = true;
							   scene.low_res_mode = true;
						   }
					   }
					   else if (to_seconds(clock::now() - low_res_mode_time) >= 1.0f && win.low_res_mode)
					   {
						   win.low_res_mode	  = false;
						   scene.low_res_mode = false;
						   dirty			  = true;
					   }

					   backbuffer_dirty = dirty;
					   dirty			= false;
					   return !should_quit;
				   },

				   .render = [&](image_view pixels) noexcept //
				   {										 //
					   ray_tracer->render(scene, pixels, threads);
				   } });
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
		std::cerr << "FATAL ERROR: " << ex.what() << "\n";
		window::error_message_box("FATAL ERROR", ex.what());
		return 1;
	}
	catch (...)
	{
		std::cerr << "FATAL ERROR: An unspecified error occurred.\n";
		window::error_message_box("FATAL ERROR", "An unspecified error occurred.");
		return 1;
	}

	return 0;
}
