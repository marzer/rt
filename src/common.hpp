#pragma once
#include <muu/preprocessor.h>
MUU_DISABLE_WARNINGS;
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <utility>
#include <span>
#include <array>
#include <muu/matrix.h>
#include <muu/vector.h>
#include <muu/scope_guard.h>
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

	using vec2u = muu::vector<uint32_t, 2>;
	using vec3	= muu::vector<float, 3>;
}
