#pragma once
#include "common.hpp"
#include "back_buffer.hpp"
MUU_DISABLE_WARNINGS;
#include <functional>
MUU_ENABLE_WARNINGS;

namespace rt
{
	struct window_events
	{
		std::function<void(int)> key_down;
		std::function<void(int)> key_up;
		std::function<bool(float /* delta_time */, bool& /* backbuffer_dirty */)> update;
		std::function<void(image_view)> render;
	};

	class window
	{
	  private:
		vec2u size_								 = {};
		std::array<void*, 2> handles_			 = {};
		std::array<back_buffer, 2> back_buffers_ = {};

	  public:
		bool low_res_mode = false;

		window() noexcept = default;

		MUU_NODISCARD_CTOR
		window(std::string_view, vec2u);

		MUU_NODISCARD_CTOR
		window(window&&) noexcept;

		window& operator=(window&&) noexcept;

		~window() noexcept;

		MUU_PURE_INLINE_GETTER
		explicit operator bool() const noexcept
		{
			return handles_[0]										  //
				&& handles_[1]										  //
				&& back_buffers_[static_cast<unsigned>(low_res_mode)] //
				&& size_.x > 0										  //
				&& size_.y > 0;
		}

		void loop(const window_events& ev);

		static void error_message_box(const char* title, const char* msg, const window* = nullptr) noexcept;
	};

	static_assert(!std::is_copy_constructible_v<window>);
	static_assert(!std::is_copy_assignable_v<window>);
}
