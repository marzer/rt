#include "image.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/aligned_alloc.h>
MUU_ENABLE_WARNINGS;

using namespace rt;

image::image(vec2u sz) noexcept //
	: data_{ static_cast<pixel_type*>(
		sz.x * sz.y ? muu::aligned_alloc(sz.x * sz.y * sizeof(pixel_type), buffer_alignment) : nullptr) },
	  size_{ sz }
{}

image::image(image&& other) noexcept //
	: data_{ std::exchange(other.data_, {}) },
	  size_{ std::exchange(other.size_, {}) }
{}

image& image::operator=(image&& rhs) noexcept
{
	data_ = std::exchange(rhs.data_, {});
	size_ = std::exchange(rhs.size_, {});
	return *this;
}

image::~image() noexcept
{
	if (data_)
		muu::aligned_free(data_);
}
