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
#include <muu/matrix.h>
#include <muu/vector.h>
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

	using vec2u = muu::vector<unsigned, 2>;
	using vec2	= muu::vector<float, 2>;
	using vec3	= muu::vector<float, 3>;

	struct scene;
	struct window_events;
	class window;
	class image_view;

	// soa:
	class spheres;
	class vertices;
	class boxes;
	class cones;
	class planes;
}
