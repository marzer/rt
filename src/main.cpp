#include "common.hpp"
#include "window.hpp"
MUU_DISABLE_WARNINGS;
#include <iostream>
#include <exception>
#include <SDL_main.h>
MUU_ENABLE_WARNINGS;

void run()
{
	rt::window::subsystem_initialize();
	const auto shutdown = muu::scope_guard{ rt::window::subsystem_shutdown };

	auto window = rt::window{ "rt", { 800, 600 } };

	window.loop(
		[](std::span<uint32_t> pixels) noexcept
		{
			for (auto& pixel : pixels)
				pixel = 0xFF0000FF;
		});
}

int main(int /*argc*/, char** /*argv*/)
{
	try
	{
		run();
	}
	catch (const std::exception& ex)
	{
		std::cerr << "FATAL ERROR: " << ex.what() << "\n";
		rt::window::error_message_box("FATAL ERROR", ex.what());
		return 1;
	}
	catch (...)
	{
		std::cerr << "FATAL ERROR: An unspecified error occurred.\n";
		rt::window::error_message_box("FATAL ERROR", "An unspecified error occurred.");
		return 1;
	}

	return 0;
}
