#include "random.hpp"
MUU_DISABLE_WARNINGS;
#include <random>
MUU_ENABLE_WARNINGS;

namespace
{
	MUU_NODISCARD
	static auto& random_engine() noexcept
	{
		MUU_DISABLE_WARNINGS;
		thread_local std::random_device rdev;
		thread_local std::mt19937 engine{ rdev() };
		MUU_ENABLE_WARNINGS;

		return engine;
	}
}

namespace rt::detail
{
	float random_float() noexcept
	{
		thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
		return distribution(random_engine());
	}
}
