#pragma once
#include <muu/preprocessor.h>
MUU_DISABLE_SPAM_WARNINGS;
MUU_DISABLE_WARNINGS;
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <utility>
#include <array>
#include <string_view>
#include <muu/vector.h>
#include <muu/quaternion.h>
#include <muu/matrix.h>
#include <muu/plane.h>
#include <muu/scope_guard.h>
#include <muu/assume_aligned.h>
MUU_ENABLE_WARNINGS;

namespace rt
{
	MUU_DISABLE_WARNINGS;
	using namespace muu::literals;
	using namespace std::string_view_literals;
	MUU_ENABLE_WARNINGS;

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

	using vec2u	 = muu::vector<unsigned, 2>;
	using vec2	 = muu::vector<float, 2>;
	using vec3	 = muu::vector<float, 3>;
	using vec4	 = muu::vector<float, 4>;
	using quat	 = muu::quaternion<float>;
	using mat3	 = muu::matrix<float, 3, 3>;
	using mat4	 = muu::matrix<float, 4, 4>;
	using plane	 = muu::plane<float>;
	using floats = muu::constants<float>;

	struct scene;
	struct events;
	struct viewport;

	class window;
	class image_view;
	class camera;

	// soa:
	class planes;
	class spheres;
	class boxes;

	class MUU_ABSTRACT_INTERFACE ray_tracer_interface
	{
	  public:
		virtual void render(const scene&, image_view&, muu::thread_pool&) noexcept = 0;

		virtual ~ray_tracer_interface() noexcept = default;
	};

	class scalar_ray_tracer;
	class simd_ray_tracer;
}
