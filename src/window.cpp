#include "window.hpp"
#include "image.hpp"
MUU_DISABLE_WARNINGS;
#include <stdexcept>
#include <SDL.h>
#include <string>
#include <atomic>
#include <mutex>
MUU_ENABLE_WARNINGS;

using namespace rt;

namespace
{
	static std::mutex sdl_mx;
	static std::atomic_bool sdl_initialized = false;

	void sdl_shutdown() noexcept
	{
		if (!sdl_initialized)
			return;

		const auto lock = std::scoped_lock{ sdl_mx };
		if (!sdl_initialized)
			return;

		SDL_Quit();
		sdl_initialized = false;
	}

	void sdl_initialize()
	{
		if (sdl_initialized)
			return;

		const auto lock = std::scoped_lock{ sdl_mx };
		if (sdl_initialized)
			return;

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
			throw std::runtime_error{ SDL_GetError() };
		sdl_initialized = true;

		std::atexit([]() { sdl_shutdown(); });
		std::at_quick_exit([]() { sdl_shutdown(); });
	}

}

window::window(std::string_view title, vec2u size) //
	: size_{ size }
{
	sdl_initialize();

	handles_[0] = SDL_CreateWindow(std::string(title).c_str(),
								   SDL_WINDOWPOS_CENTERED,
								   SDL_WINDOWPOS_CENTERED,
								   static_cast<int>(size.x),
								   static_cast<int>(size.y),
								   SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	if (!handles_[0])
		throw std::runtime_error{ SDL_GetError() };

	handles_[1] = SDL_CreateRenderer(static_cast<SDL_Window*>(handles_[0]), -1, SDL_RENDERER_ACCELERATED);
	if (!handles_[1])
		throw std::runtime_error{ SDL_GetError() };

	back_buffers_[0] = back_buffer{ size, handles_[1] };
	back_buffers_[1] = back_buffer{ vec2u{ vec2{ size } * 0.1f }, handles_[1] };
}

window::window(window&& other) noexcept //
	: size_{ std::exchange(other.size_, {}) },
	  handles_{ std::exchange(other.handles_, {}) },
	  back_buffers_{ std::move(other.back_buffers_) },
	  low_res_mode{ std::exchange(low_res_mode, {}) }
{}

window& window::operator=(window&& rhs) noexcept
{
	size_		  = std::exchange(rhs.size_, {});
	handles_	  = std::exchange(rhs.handles_, {});
	back_buffers_ = std::move(rhs.back_buffers_);
	low_res_mode  = std::exchange(rhs.low_res_mode, {});
	return *this;
}

window::~window() noexcept
{
	back_buffers_ = {};

	if (handles_[1])
		SDL_DestroyRenderer(static_cast<SDL_Renderer*>(handles_[1]));

	if (handles_[0])
		SDL_DestroyWindow(static_cast<SDL_Window*>(handles_[0]));
}

void window::loop(const window_events& ev)
{
	auto prev_time = clock::now();

	while (true)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_APP_TERMINATING: [[fallthrough]];
				case SDL_QUIT: return;

				case SDL_KEYDOWN:
					if (ev.key_down)
						ev.key_down(e.key.keysym.sym);
					break;

				case SDL_KEYUP:
					if (ev.key_up)
						ev.key_up(e.key.keysym.sym);
					break;
			}
		}

		auto time	  = clock::now();
		const auto dt = time - prev_time;
		prev_time	  = time;

		bool backbuffer_dirty = true;
		if (ev.update)
		{
			if (!ev.update(std::min(to_seconds(dt), 0.25f), backbuffer_dirty))
				return;
		}

		auto& current_back_buffer = back_buffers_[static_cast<unsigned>(low_res_mode)];

		if (ev.render && current_back_buffer && backbuffer_dirty)
		{
			ev.render(current_back_buffer.image());
			current_back_buffer.flush();
		}

		if (current_back_buffer)
		{
			SDL_RenderClear(static_cast<SDL_Renderer*>(handles_[1]));

			SDL_RenderCopy(static_cast<SDL_Renderer*>(handles_[1]),
						   static_cast<SDL_Texture*>(current_back_buffer.handle()),
						   nullptr,
						   nullptr);

			SDL_RenderPresent(static_cast<SDL_Renderer*>(handles_[1]));
		}
	}
}

void window::error_message_box(const char* title, const char* msg, const window* parent) noexcept
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
							 title,
							 msg,
							 parent && *parent ? static_cast<SDL_Window*>(parent->handles_[0]) : nullptr);
}
