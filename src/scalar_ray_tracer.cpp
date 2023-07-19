#include "scalar_ray_tracer.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "colour.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/thread_pool.h>
#include <muu/bounding_sphere.h>
#include <muu/ray.h>
MUU_ENABLE_WARNINGS;

MUU_FORCE_NDEBUG_OPTIMIZATIONS;

using namespace rt;

namespace
{
	// MUU_PURE_GETTER
	// static vec3 MUU_VECTORCALL lambert(const trace_result& r) noexcept
	//{
	//	return 0.5 * color(r.normal.x + 1, r.normal.y + 1, r.normal.z + 1);
	// }

	struct hit_result
	{
		float distance;
		vec3 normal;
		rt::colour colour;

		MUU_PURE_INLINE_GETTER
		explicit constexpr operator bool() const noexcept
		{
			return distance >= 0.0f;
		}
	};

	MUU_PURE_GETTER
	static hit_result MUU_VECTORCALL test_planes(const rt::scene& scene, const rt::ray r) noexcept
	{
		MUU_FMA_BLOCK;

		std::optional<size_t> hit_index;
		float hit_dist{};

		for (size_t i = 0; i < scene.planes.size(); i++)
		{
			const auto obj = scene.planes.value()[i];
			const auto hit = r.hits(obj);
			if (!hit || (hit_index && hit_dist <= *hit))
				continue;

			hit_index = i;
			hit_dist  = *hit;
		}

		if (!hit_index)
			return { -1 };

		return hit_result{ .distance = hit_dist,
						   .normal	 = scene.planes.value()[*hit_index].normal,
						   .colour	 = scene.materials.colour()[scene.planes.material()[*hit_index]] };
	}

	MUU_PURE_GETTER
	static hit_result MUU_VECTORCALL test_spheres(const rt::scene& scene, const ray r) noexcept
	{
		MUU_FMA_BLOCK;

		std::optional<size_t> hit_index;
		float hit_dist{};

		for (size_t i = 0; i < scene.spheres.size(); i++)
		{
			const auto obj = scene.spheres.value()[i];
			const auto hit = r.hits(obj);
			if (!hit || (hit_index && hit_dist <= *hit))
				continue;

			hit_index = i;
			hit_dist  = *hit;
		}

		if (!hit_index)
			return { -1 };

		return hit_result{ .distance = hit_dist,
						   .normal	 = vec3::direction(scene.spheres.value()[*hit_index].center, r.at(hit_dist)),
						   .colour	 = scene.materials.colour()[scene.spheres.material()[*hit_index]] };
	}

	MUU_PURE_GETTER
	static hit_result MUU_VECTORCALL test_boxes(const rt::scene& /*scene*/, const ray /*r*/) noexcept
	{
		return { -1 };
	}

	MUU_PURE_GETTER
	static hit_result select(const hit_result& a, const hit_result& b) noexcept
	{
		if (!a)
			return b;

		return !b || a.distance <= b.distance ? a : b;
	}

	[[nodiscard]]
	static colour MUU_VECTORCALL trace(const rt::scene& scene, const ray r, unsigned max_bounces) noexcept
	{
		MUU_ASSUME(max_bounces);

		auto hit = test_planes(scene, r);
		hit		 = select(test_spheres(scene, r), hit);
		hit		 = select(test_boxes(scene, r), hit);
		if (!hit)
		{
#if 1
			return colour{
				muu::lerp(colours::white.xyzw, vec4{ 0.5f, 0.7f, 1.0f, 1.0f }, 0.5f * (r.direction.y + 1.0f))
			};
#else
			return {};
#endif
		}

		if (!(--max_bounces))
			return hit.colour;

		auto col =
			trace(scene, r.bounce(hit.distance, vec3::normalize(hit.normal + random_unit_vector())), max_bounces);
		col.xyz *= 0.5f;
		return col;
	}
}

void MUU_VECTORCALL scalar_ray_tracer::render(const rt::scene& scene,
											  image_view& pixels,
											  muu::thread_pool& threads) noexcept
{
	const auto view	   = scene.camera.viewport(pixels.size());
	const auto samples = scene.low_res_mode ? std::min(scene.samples_per_pixel, 5u) : scene.samples_per_pixel;
	const auto bounces = scene.low_res_mode ? std::min(scene.max_bounces, 5u) : scene.max_bounces;

	const auto worker = [=, &scene](unsigned pixel_index) noexcept
	{
		const auto screen_pos = pixels.position_of(pixel_index);

		rt::colour colour{};
		for (unsigned i = 0, e = samples; i < e; i++)
		{
			const auto pos	= vec2{ screen_pos } + random<vec2>();
			const auto near = view.screen_to_world(pos, 0.0f);
			const auto far	= view.screen_to_world(pos, 1.0f);

			colour += trace(scene, ray{ near, vec3::direction(near, far) }, bounces);
		}
		colour.xyz /= static_cast<float>(scene.samples_per_pixel);
		colour.x = std::sqrt(colour.x);
		colour.y = std::sqrt(colour.y);
		colour.z = std::sqrt(colour.z);

		pixels(screen_pos) = static_cast<uint32_t>(colour);
	};

	threads.for_range(unsigned{}, pixels.size().x * pixels.size().y, worker);
	threads.wait();
}
