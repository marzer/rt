#pragma once
#include "common.hpp"
#include "soa.hpp"

namespace rt
{
	struct scene
	{
		rt::spheres spheres;
		// rt::planes planes;
		// rt::boxes boxes;
		// rt::cones cones;

		MUU_NODISCARD
		static scene load(std::string_view file);
	};
}
