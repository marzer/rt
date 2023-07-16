#pragma once
#include "common.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/function_view.h>
MUU_ENABLE_WARNINGS;

namespace rt
{
	class window
	{
	  private:
		vec2u size_;
		std::array<void*, 3> handles_ = {};

	  public:
		window() noexcept = default;
		window(const char*, vec2u);
		window(window&&) noexcept;
		window& operator=(window&&) noexcept;
		~window() noexcept;

		void loop(muu::function_view<void(std::span<uint32_t>)>);

		explicit constexpr operator bool() const noexcept
		{
			for (auto handle : handles_)
				if (!handle)
					return false;
			return true;
		}

		static void subsystem_initialize();
		static void subsystem_shutdown() noexcept;
		static void error_message_box(const char* title, const char* msg, const window* = nullptr) noexcept;
	};

	static_assert(!std::is_copy_constructible_v<window>);
	static_assert(!std::is_copy_assignable_v<window>);
}
