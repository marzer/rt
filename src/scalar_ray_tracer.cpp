#include "scalar_ray_tracer.hpp"
#include "scene.hpp"
#include "image.hpp"
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
		vec3 colour;
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
	static std::optional<hit_result> MUU_VECTORCALL test(const rt::planes& planes, const rt::ray ray) noexcept
	{
		MUU_FMA_BLOCK;

		std::optional<size_t> hit_index;
		float hit_dist{};

		for (size_t i = 0; i < planes.size(); i++)
		{
			const auto plane = planes.value()[i];
			const auto hit	 = ray.hits(plane);
			if (!hit || (hit_index && hit_dist <= *hit))
				continue;

			hit_index = i;
			hit_dist  = *hit;
		}

		if (!hit_index)
			return {};

		return hit_result{
			.trace	  = { .normal = planes.value()[*hit_index].normal, //
						  .colour = { 1.0f, 0.0f, 0.0f } },
			.distance = hit_dist,
		};
	}

	MUU_PURE_GETTER
	static std::optional<hit_result> MUU_VECTORCALL test(const rt::spheres& /*spheres*/, const rt::ray /*ray*/) noexcept
	{
		return {};
	}

	MUU_PURE_GETTER
	static std::optional<hit_result> MUU_VECTORCALL test(const rt::boxes& /*boxes*/, const rt::ray /*ray*/) noexcept
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
		auto hit = test(scene.planes, ray);
		hit		 = select(test(scene.spheres, ray), hit);
		hit		 = select(test(scene.boxes, ray), hit);
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
		const auto near		  = view.screen_to_world(vec2{ screen_pos } + vec2{ 0.5f }, 0.0f);
		const auto far		  = view.screen_to_world(vec2{ screen_pos } + vec2{ 0.5f }, 1.0f);

		vec3 fcolour{};
		for (unsigned i = 0, e = scene.samples_per_pixel; i < e; i++)
		{
			if (const auto trace = test(scene, rt::ray{ near, vec3::direction(near, far) }))
				fcolour += trace->colour;
		}
		fcolour /= static_cast<float>(scene.samples_per_pixel);
		fcolour = vec3::clamp(fcolour, vec3::constants::zero, vec3::constants::one);
		fcolour *= vec3{ 255.99999f };
		const auto colour = vec3u{ fcolour };

		pixels(screen_pos) = (colour.x << 24u) | (colour.y << 16u) | (colour.z << 8u) | 0xFFu;
	};

	threads.for_range(unsigned{}, pixels.size().x * pixels.size().y, worker);
	threads.wait();
}
