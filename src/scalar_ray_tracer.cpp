#include "scalar_ray_tracer.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "colour.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/thread_pool.h>
#include <muu/bounding_sphere.h>
#include <muu/ray.h>
MUU_ENABLE_WARNINGS;

using namespace rt;

namespace
{
	struct trace_result
	{
		vec3 normal;
		rt::colour colour;
	};

	// MUU_PURE_GETTER
	// static vec3 MUU_VECTORCALL lambert(const trace_result& r) noexcept
	//{
	//	return 0.5 * color(r.normal.x + 1, r.normal.y + 1, r.normal.z + 1);
	// }

	struct hit_result
	{
		trace_result trace;
		float distance;
	};

	MUU_PURE_GETTER
	static std::optional<hit_result> MUU_VECTORCALL test_planes(const rt::scene& scene, const rt::ray ray) noexcept
	{
		MUU_FMA_BLOCK;

		std::optional<size_t> hit_index;
		float hit_dist{};

		for (size_t i = 0; i < scene.planes.size(); i++)
		{
			const auto obj = scene.planes.value()[i];
			const auto hit = ray.hits(obj);
			if (!hit || (hit_index && hit_dist <= *hit))
				continue;

			hit_index = i;
			hit_dist  = *hit;
		}

		if (!hit_index)
			return {};

		return hit_result{
			.trace	  = { .normal = scene.planes.value()[*hit_index].normal, //
						  .colour = scene.materials.colour()[scene.planes.material()[*hit_index]] },
			.distance = hit_dist,
		};
	}

	MUU_PURE_GETTER
	static std::optional<hit_result> MUU_VECTORCALL test_spheres(const rt::scene& scene, const rt::ray ray) noexcept
	{
		MUU_FMA_BLOCK;

		std::optional<size_t> hit_index;
		float hit_dist{};

		for (size_t i = 0; i < scene.spheres.size(); i++)
		{
			const auto obj = scene.spheres.value()[i];
			const auto hit = ray.hits(obj);
			if (!hit || (hit_index && hit_dist <= *hit))
				continue;

			hit_index = i;
			hit_dist  = *hit;
		}

		if (!hit_index)
			return {};

		return hit_result{
			.trace	  = { .normal = vec3::direction(scene.spheres.value()[*hit_index].center,
													ray.origin + ray.direction * hit_dist), //
						  .colour = scene.materials.colour()[scene.spheres.material()[*hit_index]] },
			.distance = hit_dist,
		};
	}

	MUU_PURE_GETTER
	static std::optional<hit_result> MUU_VECTORCALL test_boxes(const rt::scene& /*scene*/,
															   const rt::ray /*ray*/) noexcept
	{
		return {};
	}

	MUU_PURE_GETTER
	static std::optional<hit_result> select(const std::optional<hit_result>& a,
											const std::optional<hit_result>& b) noexcept
	{
		if (!a)
			return b;

		return !b || a->distance <= b->distance ? a : b;
	}

	MUU_PURE_GETTER
	static std::optional<trace_result> MUU_VECTORCALL test(const rt::scene& scene, const rt::ray ray) noexcept
	{
		auto hit = test_planes(scene, ray);
		hit		 = select(test_spheres(scene, ray), hit);
		hit		 = select(test_boxes(scene, ray), hit);
		if (hit)
			return hit->trace;
		return {};
	}
}

void MUU_VECTORCALL scalar_ray_tracer::render(const rt::scene& scene,
											  image_view& pixels,
											  muu::thread_pool& threads) noexcept
{
	const auto view = scene.camera.viewport(pixels.size());

	const auto worker = [&, pixels, view](unsigned pixel_index) noexcept //
	{
		const auto screen_pos = pixels.position_of(pixel_index);

		rt::colour colour{};
		for (unsigned i = 0, e = scene.samples_per_pixel; i < e; i++)
		{
			const auto pos	= vec2{ screen_pos } + vec2{ random_float(), random_float() };
			const auto near = view.screen_to_world(pos, 0.0f);
			const auto far	= view.screen_to_world(pos, 1.0f);

			if (const auto trace = test(scene, rt::ray{ near, vec3::direction(near, far) }))
				colour.xyzw += trace->colour.xyzw;
		}
		colour.xyzw /= static_cast<float>(scene.samples_per_pixel);
		pixels(screen_pos) = static_cast<uint32_t>(colour);
	};

	threads.for_range(unsigned{}, pixels.size().x * pixels.size().y, worker);
	threads.wait();
}
