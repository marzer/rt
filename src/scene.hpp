#pragma once
#include "common.hpp"
#include "camera.hpp"
#include "soa.hpp"

namespace rt
{
	struct scene
	{
		unsigned samples_per_pixel = 30;
		unsigned max_bounces	   = 10;
		bool low_res_mode		   = false;

		rt::camera camera;
		rt::materials materials;
		rt::planes planes;
		rt::spheres spheres;
		rt::boxes boxes;

		MUU_NODISCARD
		static scene load(std::string_view file);
	};
}
