#include "common.hpp"
MUU_DISABLE_WARNINGS;
#include <random>
MUU_ENABLE_WARNINGS;

namespace
{
	MUU_NODISCARD
	static auto& random_engine() noexcept
	{
		thread_local std::random_device rdev;
		thread_local std::mt19937 engine{ rdev() };
		return engine;
	}
}

namespace rt::detail
{
	float MUU_VECTORCALL random_float() noexcept
	{
		thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
		return distribution(random_engine());
	}
}
