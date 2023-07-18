#pragma once
#include "image.hpp"
namespace rt
{
	class back_buffer
	{
	  private:
		rt::image img_;
		void* handle_ = {};

	  public:
		MUU_NODISCARD_CTOR
		back_buffer() noexcept = default;

		MUU_NODISCARD_CTOR
		back_buffer(vec2u sz, void* target = nullptr);

		MUU_NODISCARD_CTOR
		back_buffer(back_buffer&&) noexcept;

		back_buffer& operator=(back_buffer&&) noexcept;

		~back_buffer() noexcept;

		MUU_PURE_INLINE_GETTER
		explicit operator bool() const noexcept
		{
			return img_ && handle_;
		}

		MUU_PURE_INLINE_GETTER
		rt::image& image() noexcept
		{
			return img_;
		}

		MUU_PURE_INLINE_GETTER
		void* handle() noexcept
		{
			return handle_;
		}

		void flush();
	};

	static_assert(!std::is_copy_constructible_v<back_buffer>);
	static_assert(!std::is_copy_assignable_v<back_buffer>);
}
