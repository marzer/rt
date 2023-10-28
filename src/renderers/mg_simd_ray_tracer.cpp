#include "../scene.hpp"
#include "../image.hpp"
#include "../renderer.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/thread_pool.h>
#include <xsimd/xsimd.hpp>
MUU_ENABLE_WARNINGS;

using namespace rt;

namespace
{
	struct mg_simd_ray_tracer final : renderer_interface
	{
		void MUU_VECTORCALL render(const rt::scene& /*scene*/,
								   image_view& /*pixels*/,
								   muu::thread_pool& /*threads*/) noexcept override
		{
			// todo
		}
	};

	REGISTER_RENDERER(mg_simd_ray_tracer);
}
