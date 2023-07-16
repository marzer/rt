#include "window.hpp"
#include <stdexcept>
MUU_DISABLE_WARNINGS;
#include <SDL.h>
MUU_ENABLE_WARNINGS;

using namespace rt;

window::window(const char* title, vec2u size)
	: size_{ size },
	  handles_{
		  SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, SDL_WINDOW_OPENGL)
	  }
{
	if (!handles_[0])
		throw std::runtime_error{ SDL_GetError() };

	handles_[1] = SDL_CreateRenderer(static_cast<SDL_Window*>(handles_[0]), -1, SDL_RENDERER_ACCELERATED);
	if (!handles_[1])
		throw std::runtime_error{ SDL_GetError() };

	handles_[2] = SDL_CreateTexture(static_cast<SDL_Renderer*>(handles_[1]),
									SDL_PIXELFORMAT_RGBA8888,
									SDL_TEXTUREACCESS_STREAMING,
									size.x,
									size.y);
}

window::window(window&& other) noexcept //
	: size_{ std::exchange(other.size_, {}) },
	  handles_{ std::exchange(other.handles_, {}) }
{}

window& window::operator=(window&& rhs) noexcept
{
	size_	 = std::exchange(rhs.size_, {});
	handles_ = std::exchange(rhs.handles_, {});
	return *this;
}

window::~window() noexcept
{
	if (handles_[2])
		SDL_DestroyTexture(static_cast<SDL_Texture*>(handles_[2]));

	if (handles_[1])
		SDL_DestroyRenderer(static_cast<SDL_Renderer*>(handles_[1]));

	if (handles_[0])
		SDL_DestroyWindow(static_cast<SDL_Window*>(handles_[0]));
}

void window::loop(muu::function_view<void(std::span<uint32_t>)> func)
{
	while (true)
	{
		if (SDL_Event e; SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				break;
		}

		if (func)
		{
			void* pixels{};
			int pitch{};
			if (SDL_LockTexture(static_cast<SDL_Texture*>(handles_[2]), nullptr, &pixels, &pitch) < 0)
				throw std::runtime_error{ SDL_GetError() };
			const auto unlock =
				muu::scope_guard{ [&]() noexcept { SDL_UnlockTexture(static_cast<SDL_Texture*>(handles_[2])); } };

			func(std::span<uint32_t>{ static_cast<uint32_t*>(pixels), size_.x * size_.y });
		}

		SDL_RenderClear(static_cast<SDL_Renderer*>(handles_[1]));

		SDL_RenderCopy(static_cast<SDL_Renderer*>(handles_[1]),
					   static_cast<SDL_Texture*>(handles_[2]),
					   nullptr,
					   nullptr);

		SDL_RenderPresent(static_cast<SDL_Renderer*>(handles_[1]));
	}
}

void window::subsystem_initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
		throw std::runtime_error{ SDL_GetError() };
}

void window::subsystem_shutdown() noexcept
{
	SDL_Quit();
}

void window::error_message_box(const char* title, const char* msg, const window* parent) noexcept
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
							 title,
							 msg,
							 parent && *parent ? static_cast<SDL_Window*>(parent->handles_[0]) : nullptr);
}
