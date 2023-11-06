#include "back_buffer.hpp"
MUU_DISABLE_WARNINGS;
#include <SDL.h>
#include <stdexcept>
#include <cstring> // memcpy
MUU_ENABLE_WARNINGS;

using namespace rt;

back_buffer::back_buffer(vec2u sz, void* target) //
	: img_{ sz }
{
	handle_ = SDL_CreateTexture(static_cast<SDL_Renderer*>(target),
								SDL_PIXELFORMAT_RGBA8888,
								SDL_TEXTUREACCESS_STREAMING,
								static_cast<int>(sz.x),
								static_cast<int>(sz.y));
	if (!handle_)
		throw std::runtime_error{ SDL_GetError() };
}

back_buffer::back_buffer(back_buffer&& other) noexcept
	: img_{ std::move(other.img_) },
	  handle_{ std::exchange(other.handle_, {}) }
{}

back_buffer& back_buffer::operator=(back_buffer&& rhs) noexcept
{
	img_	= std::move(rhs.img_);
	handle_ = std::exchange(rhs.handle_, {});
	return *this;
}

back_buffer::~back_buffer() noexcept
{
	if (handle_)
		SDL_DestroyTexture(static_cast<SDL_Texture*>(handle_));
}

void back_buffer::flush()
{
	void* pixels{};
	int pitch{};
	if (SDL_LockTexture(static_cast<SDL_Texture*>(handle_), nullptr, &pixels, &pitch) < 0)
		throw std::runtime_error{ SDL_GetError() };
	const auto unlock = muu::scope_guard{ [&]() noexcept { SDL_UnlockTexture(static_cast<SDL_Texture*>(handle_)); } };

	assert(static_cast<size_t>(pitch) == sizeof(image::pixel_type) * img_.size().x);
	std::memcpy(pixels, img_.data(), sizeof(image::pixel_type) * img_.size().x * img_.size().y);
}
