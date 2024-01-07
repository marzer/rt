#include "../renderer.hpp"

using namespace rt;

namespace
{
	struct sm_ray_tracer final : renderer_interface
	{
		void render(const rt::scene& /*scene*/, image_view& /*pixels*/, muu::thread_pool& /*threads*/) noexcept override
		{
			//
		}
	};

	REGISTER_RENDERER(sm_ray_tracer);
}
