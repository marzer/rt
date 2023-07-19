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
#include <chrono>
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

	class MUU_ABSTRACT_INTERFACE ray_tracer_interface
	{
	  public:
		virtual void MUU_VECTORCALL render(const scene&, image_view&, muu::thread_pool&) noexcept = 0;

		virtual ~ray_tracer_interface() noexcept = default;
	};

	class scalar_ray_tracer;
	class simd_ray_tracer;

	inline namespace literals
	{
		MUU_DISABLE_WARNINGS;
		using namespace muu::literals;
		using namespace std::string_view_literals;
		MUU_ENABLE_WARNINGS;
	}

	namespace detail
	{
		template <typename>
		struct randomizer;
	}

	template <typename T>
	MUU_ALWAYS_INLINE
	static T MUU_VECTORCALL random() noexcept
	{
		return detail::randomizer<T>::get();
	}

	namespace detail
	{
		[[nodiscard]]
		float MUU_VECTORCALL random_float() noexcept;

		template <typename>
		struct randomizer;

		template <std::floating_point Float>
		struct randomizer<Float>
		{
			MUU_ALWAYS_INLINE
			static Float MUU_VECTORCALL get() noexcept
			{
				return static_cast<Float>(random_float());
			}
		};

		template <std::floating_point Float, size_t Dimensions>
		struct randomizer<muu::vector<Float, Dimensions>>
		{
			MUU_ALWAYS_INLINE
			static muu::vector<Float, Dimensions> MUU_VECTORCALL get() noexcept
			{
				if constexpr (Dimensions == 2)
					return muu::vector<Float, Dimensions>{ random<Float>(), random<Float>() };
				else if constexpr (Dimensions == 3)
					return muu::vector<Float, Dimensions>{ random<Float>(), random<Float>(), random<Float>() };
				else if constexpr (Dimensions == 4)
					return muu::vector<Float, Dimensions>{ random<Float>(),
														   random<Float>(),
														   random<Float>(),
														   random<Float>() };
			}
		};
	}

	[[nodiscard]]
	inline vec3 MUU_VECTORCALL random_unit_vector() noexcept
	{
		while (true)
		{
			const auto p = random<vec3>() - vec3::constants::one_over_two;
			if MUU_UNLIKELY(p == vec3::constants::zero)
				continue;
			return vec3::normalize(p);
		}
	}

	MUU_PURE_INLINE_GETTER
	constexpr float to_seconds(nanoseconds ns) noexcept
	{
		return std::chrono::duration<float>{ ns }.count();
	}

}
