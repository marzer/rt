#pragma once
#include "common.hpp"
#include "camera.hpp"
#include "soa.hpp"
#include <iostream>

namespace rt
{
	struct scene
	{
		unsigned samples_per_pixel = 30;
		unsigned max_bounces	   = 10;

		std::string path;
		rt::camera camera;
		rt::materials materials;
		rt::planes planes;
		rt::spheres spheres;
		rt::boxes boxes;

		MUU_NODISCARD
		static scene load(std::string_view file);

		MUU_NODISCARD
		static scene load_first_available();
	};
}
