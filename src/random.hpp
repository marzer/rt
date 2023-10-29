#pragma once
#include "common.hpp"

namespace rt
{
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
		float random_float() noexcept;

		template <typename>
		struct randomizer;

		template <std::floating_point Float>
		struct randomizer<Float>
		{
			MUU_ALWAYS_INLINE
			static Float get() noexcept
			{
				return static_cast<Float>(random_float());
			}
		};

		template <std::floating_point Float, size_t Dimensions>
		struct randomizer<muu::vector<Float, Dimensions>>
		{
			MUU_ALWAYS_INLINE
			static muu::vector<Float, Dimensions> get() noexcept
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
	inline vec3 random_unit_vector() noexcept
	{
		while (true)
		{
			const auto p = random<vec3>();
			if MUU_UNLIKELY(p == vec3::constants::zero)
				continue;
			return vec3::normalize(p);
		}
	}
}
