#pragma once
#include "colour.hpp"

namespace rt
{
	MUU_PURE_GETTER
	constexpr colour MUU_VECTORCALL lambert(vec3 surface_normal,
											vec3 direction_to_light_source,
											colour surface_color,
											float intensity = 1.0f) noexcept
	{
		return colour{ direction_to_light_source.dot(surface_normal) * surface_color.rgb * intensity };
	}

	// todo
}
