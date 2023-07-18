#pragma once
#include "common.hpp"
namespace rt
{
	class MUU_TRIVIAL_ABI image_view
	{
	  public:
		using pixel_type = uint32_t;

	  private:
		pixel_type* data_ = {};
		vec2u size_		  = {};

	  public:
		MUU_NODISCARD_CTOR
		constexpr image_view() noexcept = default;

		MUU_NODISCARD_CTOR
		constexpr image_view(const image_view&) noexcept = default;

		MUU_NODISCARD_CTOR
		image_view(pixel_type* img, vec2u sz) noexcept //
			: data_{ img },
			  size_{ sz }
		{}

		constexpr image_view& operator=(const image_view&) noexcept = default;

		~image_view() noexcept = default;

		MUU_PURE_INLINE_GETTER
		explicit operator bool() const noexcept
		{
			return data_ && size_.x > 0 && size_.y > 0;
		}

		MUU_PURE_INLINE_GETTER
		const vec2u& size() const noexcept
		{
			return size_;
		}

		MUU_PURE_INLINE_GETTER
		pixel_type* data() noexcept
		{
			return data_;
		}

		MUU_PURE_INLINE_GETTER
		const pixel_type* data() const noexcept
		{
			return data_;
		}

		MUU_PURE_INLINE_GETTER
		pixel_type& operator()(size_t x, size_t y) noexcept
		{
			return *(data_ + (y * size_.x + x));
		}

		MUU_PURE_INLINE_GETTER
		const pixel_type& operator()(size_t x, size_t y) const noexcept
		{
			return *(data_ + (y * size_.x + x));
		}
	};

	static_assert(std::is_trivially_copy_constructible_v<image_view>);
	static_assert(std::is_trivially_copy_assignable_v<image_view>);
}
