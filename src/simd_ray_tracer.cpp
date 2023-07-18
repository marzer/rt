#include "simd_ray_tracer.hpp"
#include "scene.hpp"
#include "image.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/thread_pool.h>
#include <xsimd/xsimd.hpp>
MUU_ENABLE_WARNINGS;

using namespace rt;

void MUU_VECTORCALL simd_ray_tracer::render(const rt::scene&,
											image_view& /*pixels*/,
											muu::thread_pool& /*threads*/) noexcept
{
	// todo
}
