#pragma once
#include "common.hpp"
MUU_DISABLE_WARNINGS;
#include <functional>
MUU_ENABLE_WARNINGS;

namespace rt
{
	struct events
	{
		std::function<bool()> should_quit;
		std::function<void(float)> update;
		std::function<void(image_view)> render;
	};

	class window
	{
	  private:
		vec2u size_					  = {};
		std::array<void*, 3> handles_ = {};

	  public:
		window() noexcept = default;

		MUU_NODISCARD_CTOR
		window(std::string_view, vec2u);

		MUU_NODISCARD_CTOR
		window(window&&) noexcept;

		window& operator=(window&&) noexcept;

		~window() noexcept;

		MUU_PURE_GETTER
		explicit operator bool() const noexcept;

		void loop(const events& ev);

		static void error_message_box(const char* title, const char* msg, const window* = nullptr) noexcept;
	};

	static_assert(!std::is_copy_constructible_v<window>);
	static_assert(!std::is_copy_assignable_v<window>);
}
