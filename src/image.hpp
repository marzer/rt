#pragma once
#include "common.hpp"
namespace rt
{
	class image_view;

	class image
	{
	  public:
		using pixel_type						 = uint32_t;
		static constexpr size_t buffer_alignment = 64;

	  private:
		pixel_type* data_ = {};
		vec2u size_		  = {};

	  public:
		MUU_NODISCARD_CTOR
		image() noexcept = default;

		MUU_NODISCARD_CTOR
		image(vec2u sz) noexcept;

		MUU_NODISCARD_CTOR
		image(image&&) noexcept;

		image& operator=(image&&) noexcept;

		~image() noexcept;

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
		MUU_ATTR(assume_aligned(buffer_alignment))
		pixel_type* data() noexcept
		{
			return muu::assume_aligned<buffer_alignment>(data_);
		}

		MUU_PURE_INLINE_GETTER
		MUU_ATTR(assume_aligned(buffer_alignment))
		const pixel_type* data() const noexcept
		{
			return muu::assume_aligned<buffer_alignment>(data_);
		}

		MUU_PURE_INLINE_GETTER
		pixel_type& operator()(unsigned x, unsigned y) noexcept
		{
			return *(data_ + (y * size_.x + x));
		}

		MUU_PURE_INLINE_GETTER
		const pixel_type& operator()(unsigned x, unsigned y) const noexcept
		{
			return *(data_ + (y * size_.x + x));
		}

		MUU_PURE_INLINE_GETTER
		pixel_type& operator()(vec2u pos) noexcept
		{
			return (*this)(pos.x, pos.y);
		}

		MUU_PURE_INLINE_GETTER
		const pixel_type& operator()(vec2u pos) const noexcept
		{
			return (*this)(pos.x, pos.y);
		}

		MUU_PURE_INLINE_GETTER
		vec2u position_of(unsigned idx) const noexcept
		{
			return { (idx % size_.x), (idx / size_.x) };
		}

		image& clear(uint32_t colour) noexcept;
	};

	static_assert(!std::is_copy_constructible_v<image>);
	static_assert(!std::is_copy_assignable_v<image>);

	class MUU_TRIVIAL_ABI image_view
	{
	  public:
		using pixel_type = image::pixel_type;

	  private:
		pixel_type* data_ = {};
		vec2u size_		  = {};

	  public:
		MUU_NODISCARD_CTOR
		constexpr image_view() noexcept = default;

		MUU_NODISCARD_CTOR
		image_view(pixel_type* img, vec2u sz) noexcept //
			: data_{ img },
			  size_{ sz }
		{}

		MUU_NODISCARD_CTOR
		image_view(image& img) noexcept //
			: data_{ img.data() },
			  size_{ img.size() }
		{}

		MUU_NODISCARD_CTOR
		constexpr image_view(const image_view&) noexcept = default;

		constexpr image_view& operator=(const image_view&) noexcept = default;

		~image_view() noexcept = default;

		MUU_PURE_INLINE_GETTER
		explicit constexpr operator bool() const noexcept
		{
			return data_ && size_.x > 0 && size_.y > 0;
		}

		MUU_PURE_INLINE_GETTER
		constexpr const vec2u& size() const noexcept
		{
			return size_;
		}

		MUU_PURE_INLINE_GETTER
		constexpr pixel_type* data() const noexcept
		{
			return data_;
		}

		MUU_PURE_INLINE_GETTER
		constexpr pixel_type& operator()(unsigned x, unsigned y) const noexcept
		{
			return *(data_ + (y * size_.x + x));
		}

		MUU_PURE_INLINE_GETTER
		constexpr pixel_type& operator()(vec2u pos) const noexcept
		{
			return (*this)(pos.x, pos.y);
		}

		MUU_PURE_INLINE_GETTER
		constexpr vec2u position_of(unsigned idx) const noexcept
		{
			return { (idx % size_.x), (idx / size_.x) };
		}

		image_view& clear(uint32_t colour) noexcept;
	};

	static_assert(std::is_trivially_copy_constructible_v<image_view>);
	static_assert(std::is_trivially_copy_assignable_v<image_view>);
}
