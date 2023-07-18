#include "common.hpp"
#include "window.hpp"
#include "image_view.hpp"
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
}

void run([[maybe_unused]] std::span<const char*> args)
{
	const auto scene = scene::load(args.empty() ? "" : args[0]);

	muu::thread_pool threads;

	std::unique_ptr<ray_tracer_interface> ray_tracer{ new scalar_ray_tracer };

	auto win = window{ "rt", { 800, 600 } };
	win.loop({ .should_quit = []() noexcept { return should_quit.load(); },

			   .render =
				   [&](image_view pixels) noexcept
			   {
#ifndef NDEBUG
				   for (unsigned y = 0; y < pixels.size().y; y++)
				   {
					   for (unsigned x = 0; x < pixels.size().x; x++)
					   {
						   vec2 red_green{ vec2u{ x, y } };
						   if (red_green.length() <= 20.0)
						   {
							   pixels(x, y) = 0x00FFFFFFu;
							   continue;
						   }

						   red_green /= vec2{ pixels.size() };
						   red_green *= 255.9999f;
						   pixels(x, y) = 0x000000FFu | (static_cast<unsigned>(red_green[0]) << 24)
										| (static_cast<unsigned>(red_green[1]) << 16);
					   }
				   }
#endif

				   ray_tracer->render(scene, pixels, threads);
			   } });
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
