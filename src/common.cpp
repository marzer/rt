#include "common.hpp"
MUU_DISABLE_WARNINGS;
#include <random>
MUU_ENABLE_WARNINGS;

namespace
{
	MUU_NODISCARD
	static std::random_device& random_device() noexcept
	{
		thread_local std::random_device rdev;
		return rdev;
	}

	MUU_NODISCARD
	static std::mt19937& mersenne_twister() noexcept
	{
		thread_local std::mt19937 engine{ random_device()() };
		return engine;
	}
}

namespace rt
{
	float MUU_VECTORCALL random_float() noexcept
	{
		thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
		return distribution(mersenne_twister());
	}
}
