#include "simd_ray_tracer.hpp"
#include "scene.hpp"
#include "image_view.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/thread_pool.h>
MUU_ENABLE_WARNINGS;

using namespace rt;

void simd_ray_tracer::render(const scene&, image_view&, muu::thread_pool&) noexcept
{}
