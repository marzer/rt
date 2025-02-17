#include "window.hpp"
#include "image.hpp"
MUU_DISABLE_WARNINGS;
#include <stdexcept>
#include <SDL.h>
#include <string>
#include <atomic>
#include <mutex>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
MUU_ENABLE_WARNINGS;

using namespace rt;

#define window_handle	static_cast<SDL_Window*>(handles_[0])
#define renderer_handle static_cast<SDL_Renderer*>(handles_[1])

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

#ifdef NDEBUG
	static constexpr float low_res_factor = 0.33f;
#else
	static constexpr float low_res_factor = 0.1f;
#endif
}

static std::array<back_buffer, 2> create_back_buffers(vec2u size, SDL_Renderer* renderer)
{
	return { { back_buffer{ size, renderer }, back_buffer{ vec2u{ vec2{ size } * low_res_factor }, renderer } } };
}

window::window(std::string_view title, vec2u size) //
{
	sdl_initialize();
	SDL_Init(SDL_INIT_EVENTS);

	handles_[0] = SDL_CreateWindow(std::string(title).c_str(),
								   SDL_WINDOWPOS_CENTERED,
								   SDL_WINDOWPOS_CENTERED,
								   static_cast<int>(size.x),
								   static_cast<int>(size.y),
								   SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
	if (!handles_[0])
		throw std::runtime_error{ SDL_GetError() };

	assert(window::size() == size);

	handles_[1] = SDL_CreateRenderer(window_handle, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!handles_[1])
		throw std::runtime_error{ SDL_GetError() };

	back_buffers_ = create_back_buffers(size, renderer_handle);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(window_handle, renderer_handle);
	ImGui_ImplSDLRenderer2_Init(renderer_handle);
}

window::window(window&& other) noexcept //
	: handles_{ std::exchange(other.handles_, {}) },
	  back_buffers_{ std::move(other.back_buffers_) },
	  low_res{ std::exchange(low_res, {}) }
{}

window& window::operator=(window&& rhs) noexcept
{
	handles_	  = std::exchange(rhs.handles_, {});
	back_buffers_ = std::move(rhs.back_buffers_);
	low_res		  = std::exchange(rhs.low_res, {});
	return *this;
}

window::~window() noexcept
{
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	back_buffers_ = {};

	if (renderer_handle)
		SDL_DestroyRenderer(renderer_handle);

	if (window_handle)
		SDL_DestroyWindow(window_handle);
}

MUU_PURE
window::operator bool() const noexcept
{
	return window_handle   //
		&& renderer_handle //
		&& back_buffers_[static_cast<unsigned>(low_res)];
}

void window::loop(const window_events& ev)
{
	auto prev_time		   = clock::now();
	auto time_since_resize = clock::now();
	bool window_resized	   = false;
	unsigned new_width	   = 0;
	unsigned new_height	   = 0;
	while (true)
	{
		SDL_Event e;
		bool backbuffer_dirty = false;
		while (SDL_PollEvent(&e))
		{
			ImGui_ImplSDL2_ProcessEvent(&e);
			switch (e.type)
			{
				case SDL_APP_TERMINATING: [[fallthrough]];
				case SDL_QUIT: return;
				case SDL_KEYDOWN:
					if (e.key.repeat)
					{
						if (ev.key_held)
							ev.key_held(e.key.keysym.sym);
					}
					else
					{
						if (ev.key_down)
							ev.key_down(e.key.keysym.sym);
					}
					break;
				case SDL_KEYUP:
					if (ev.key_up)
						ev.key_up(e.key.keysym.sym);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (ev.mouse_button_down)
						ev.mouse_button_down(e.button.button,
											 static_cast<float>(e.motion.x),
											 static_cast<float>(e.motion.y));
					break;
				case SDL_MOUSEBUTTONUP:
					if (ev.mouse_button_up)
						ev.mouse_button_up(e.button.button);
					break;
				case SDL_MOUSEMOTION:
					if (ev.mouse_button_motion)
						ev.mouse_button_motion(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
					break;
				case SDL_WINDOWEVENT:
					if (e.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						time_since_resize = clock::now();
						new_width		  = static_cast<unsigned>(e.window.data1);
						new_height		  = static_cast<unsigned>(e.window.data2);
						window_resized	  = true;
					}
					break;
			}
		}

		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		auto time = clock::now();
		auto dt	  = to_seconds(time - prev_time);
		prev_time = time;
		if ((to_seconds(time - time_since_resize)) > 0.3 && window_resized)
		{
			backbuffer_dirty = true;
			window_resized	 = false;
			back_buffers_	 = create_back_buffers(vec2u{ new_width, new_height }, renderer_handle);
		}

		if (ev.update)
		{
			if (!ev.update(std::min(dt, 0.1f), backbuffer_dirty))
				return;
		}

		auto& current_back_buffer = back_buffers_[static_cast<unsigned>(low_res)];

		if (ev.render && current_back_buffer && backbuffer_dirty)
		{
			ev.render(current_back_buffer.image());
			current_back_buffer.flush();
		}

		ImGui::Render();
		SDL_RenderClear(renderer_handle);
		if (current_back_buffer)
			SDL_RenderCopy(renderer_handle, static_cast<SDL_Texture*>(current_back_buffer.handle()), nullptr, nullptr);
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer_handle);
	}
}

MUU_PURE
vec2u window::size() const noexcept
{
	int w, h;
	SDL_GetWindowSize(window_handle, &w, &h);
	return { static_cast<unsigned>(w), static_cast<unsigned>(h) };
}

MUU_PURE
std::string_view window::title() const noexcept
{
	const auto val = SDL_GetWindowTitle(window_handle);
	if (val)
		return val;
	return "";
}

void window::title(std::string_view val)
{
	if (val == title())
		return;

	if (val.empty())
	{
		SDL_SetWindowTitle(window_handle, "");
		return;
	}

	SDL_SetWindowTitle(window_handle, std::string(val).c_str());
}

MUU_PURE
bool window::get_key(int key_code) noexcept
{
	assert(key_code > 0);
	assert(key_code != SDLK_UNKNOWN);

	const auto scan_code = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key_code));
	if (scan_code <= SDL_SCANCODE_UNKNOWN || scan_code >= SDL_NUM_SCANCODES)
		return false;

	int num_keys{};
	const auto keys = SDL_GetKeyboardState(&num_keys);
	if (!keys || scan_code >= num_keys)
		return false;

	return !!keys[scan_code];
}
