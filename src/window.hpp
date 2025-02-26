#pragma once
#include "common.hpp"
#include "back_buffer.hpp"
MUU_DISABLE_WARNINGS;
#include <functional>
#include <array>
MUU_ENABLE_WARNINGS;

namespace rt
{
	struct window_events
	{
		std::function<void(int /* virtual key code */)> key_down;
		std::function<void(int /* virtual key code */)> mouse_button_up;
		std::function<void(int /* virtual key code */, float /*mouse pos x*/, float /* mouse y*/)> mouse_button_down;
		std::function<void(float /* virtual key code */, float /* virtual key code */)> mouse_button_motion;
		std::function<void(int /* virtual key code */)> key_held;
		std::function<void(int /* virtual key code */)> key_up;
		std::function<bool(float /* delta_time */, bool& /* backbuffer_dirty */)> update;

		std::function<void(image_view)> render;
	};

	class window
	{
	  private:
		std::array<void*, 2> handles_			 = {};
		std::array<back_buffer, 2> back_buffers_ = {};

		MUU_PURE_GETTER
		static bool get_key(int) noexcept;

	  public:
		bool low_res	= false;
		bool mouse_down = false; //

		window() noexcept = default;

		MUU_NODISCARD_CTOR
		window(std::string_view, vec2u);

		MUU_NODISCARD_CTOR
		window(window&&) noexcept;

		window& operator=(window&&) noexcept;

		~window() noexcept;

		MUU_PURE_GETTER
		explicit operator bool() const noexcept;

		void loop(const window_events& ev);

		MUU_PURE_GETTER
		vec2u size() const noexcept;

		MUU_PURE_GETTER
		std::string_view title() const noexcept;

		void title(std::string_view);

		template <typename T, typename... U>
		MUU_PURE_GETTER
		bool key(T key_code, U... key_codes) const noexcept
		{
			return (get_key(static_cast<int>(key_code)) || ... || get_key(static_cast<int>(key_codes)));
		}
	};

	static_assert(!std::is_copy_constructible_v<window>);
	static_assert(!std::is_copy_assignable_v<window>);
}
