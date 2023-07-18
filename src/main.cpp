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
		const auto scene = scene::load(args.empty() ? "" : args[0]);

		muu::thread_pool threads;

		std::unique_ptr<ray_tracer_interface> ray_tracer{ new scalar_ray_tracer };

		bool dirty = true;
		auto win   = window{ "rt", { 800, 600 } };
		win.loop({ .key_down =
					   [&](int key) noexcept
				   {
					   if (key == 0x20)
						   dirty = true;
				   },

				   .update = [&](float /* delta_time */, bool& backbuffer_dirty) noexcept -> bool //
				   {																			  //
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
