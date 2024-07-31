#pragma once

#if defined(__GNUC__) && __GNUC__ >= 13
	#define SOAGEN_ASSUME(...) static_cast<void>(0)
	#define MUU_ASSUME(...)	   static_cast<void>(0)
#endif

#include <muu/preprocessor.h>
MUU_DISABLE_SPAM_WARNINGS;
MUU_DISABLE_WARNINGS;
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <utility>
#include <string_view>
#include <chrono>
#include <type_traits>
#include <concepts>
#include <muu/vector.h>
#include <muu/quaternion.h>
#include <muu/matrix.h>
#include <muu/plane.h>
#include <muu/bounding_sphere.h>
#include <muu/bounding_box.h>
#include <muu/ray.h>
#include <muu/scope_guard.h>
#include <muu/assume_aligned.h>
MUU_ENABLE_WARNINGS;

namespace rt
{
	using std::size_t;
	using std::ptrdiff_t;
	using std::nullptr_t;

	using std::uint64_t;
	using std::uint32_t;
	using std::uint16_t;
	using std::uint8_t;

	using std::int64_t;
	using std::int32_t;
	using std::int16_t;
	using std::int8_t;

	using clock = std::chrono::steady_clock;
	using std::chrono::nanoseconds;
	using time_point = clock::time_point;

	using floats = muu::constants<float>;

	using vec2u	 = muu::vector<unsigned, 2>;
	using vec3u	 = muu::vector<unsigned, 3>;
	using vec4u	 = muu::vector<unsigned, 4>;
	using vec2	 = muu::vector<float, 2>;
	using vec3	 = muu::vector<float, 3>;
	using vec4	 = muu::vector<float, 4>;
	using quat	 = muu::quaternion<float>;
	using mat3	 = muu::matrix<float, 3, 3>;
	using mat4	 = muu::matrix<float, 4, 4>;
	using plane	 = muu::plane<float>;
	using sphere = muu::bounding_sphere<float>;
	using box	 = muu::bounding_box<float>;
	using ray	 = muu::ray<float>;

	struct colour;
	struct scene;
	struct viewport;
	struct window_events;
	struct renderer_interface;

	class window;
	class image;
	class image_view;
	class camera;
	class back_buffer;

	// soa:
	class materials;
	class planes;
	class spheres;
	class boxes;

	inline namespace literals
	{
		MUU_DISABLE_WARNINGS;
		using namespace muu::literals;
		using namespace std::string_view_literals;
		using namespace std::chrono_literals;
		MUU_ENABLE_WARNINGS;
	}

	MUU_PURE_INLINE_GETTER
	constexpr float to_seconds(nanoseconds ns) noexcept
	{
		return std::chrono::duration<float>{ ns }.count();
	}

	MUU_PURE_INLINE_GETTER
	constexpr vec3 reflect(vec3 v, vec3 n) noexcept
	{
		return v - 2 * vec3::dot(v, n) * n;
	}

	enum class material_type : unsigned
	{
		lambert,
		metal,
		dielectric
	};
}
