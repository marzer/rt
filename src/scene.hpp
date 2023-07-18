#pragma once
#include "common.hpp"
#include "camera.hpp"
#include "soa.hpp"

namespace rt
{
	struct scene
	{
		rt::camera camera;
		rt::planes planes;
		rt::spheres spheres;
		rt::boxes boxes;

		MUU_NODISCARD
		static scene load(std::string_view file);
	};
}
