#pragma once
#include "common.hpp"

namespace rt
{
	class scalar_ray_tracer final : public ray_tracer_interface
	{
	  public:
		void render(const scene&, image_view&, muu::thread_pool&) noexcept override;
	};
}
