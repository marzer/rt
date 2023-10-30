#include "../scene.hpp"
#include "../image.hpp"
#include "../colour.hpp"
#include "../random.hpp"
#include "../renderer.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/thread_pool.h>
#include <muu/bounding_sphere.h>
#include <muu/ray.h>
#include <array>
#include <magic_enum.hpp>
MUU_ENABLE_WARNINGS;

MUU_FORCE_NDEBUG_OPTIMIZATIONS;

using namespace rt;

namespace
{
	static constexpr float min_hit_dist = 0.001f;

	struct hit_result
	{
		float distance;
		vec3 normal;
		unsigned material;

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
			if (!hit || *hit < min_hit_dist || (hit_index && hit_dist <= *hit))
				continue;

			hit_index = i;
			hit_dist  = *hit;
		}

		if (!hit_index)
			return { -1 };

		return hit_result{ .distance = hit_dist,
						   .normal	 = scene.planes.value()[*hit_index].normal,
						   .material = scene.planes.material()[*hit_index] };
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
			if (!hit || *hit < min_hit_dist || (hit_index && hit_dist <= *hit))
				continue;

			hit_index = i;
			hit_dist  = *hit;
		}

		if (!hit_index)
			return { -1 };

		return hit_result{ .distance = hit_dist,
						   .normal	 = vec3::direction(scene.spheres.value()[*hit_index].center, r.at(hit_dist)),
						   .material = scene.spheres.material()[*hit_index] };
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

	using scatter_func = std::optional<ray> MUU_VECTORCALL(const rt::scene& scene,
														   const ray& r,
														   const hit_result& hit,
														   vec3& attenuation) noexcept;

	MUU_PURE_GETTER
	static std::optional<ray> MUU_VECTORCALL lambert_scatter(const rt::scene& scene,
															 const ray& r,
															 const hit_result& hit,
															 vec3& attenuation) noexcept
	{
		attenuation = scene.materials.albedo()[hit.material].rgb * scene.materials.reflectivity()[hit.material];

		auto scatter = hit.normal + random_unit_vector();
		if (scatter.approx_zero())
			scatter = hit.normal;
		scatter.normalize();

		return ray{ r.at(hit.distance), scatter };
	}

	MUU_PURE_GETTER
	static std::optional<ray> MUU_VECTORCALL metal_scatter(const rt::scene& scene,
														   const ray& r,
														   const hit_result& hit,
														   vec3& attenuation) noexcept
	{
		attenuation = scene.materials.albedo()[hit.material].rgb * scene.materials.reflectivity()[hit.material];

		vec3 scatter = reflect(vec3::normalize(r.direction), hit.normal)
					 + scene.materials.roughness()[hit.material] * random_unit_vector();
		if (vec3::dot(scatter, hit.normal) <= 0.0f)
			return {};

		scatter.normalize();
		return ray{ r.at(hit.distance), scatter };
	}

	static constexpr auto scatter_funcs = []() noexcept
	{
		std::array<scatter_func*, magic_enum::enum_count<material_type>()> funcs{};

		for (auto& func : funcs)
			func = lambert_scatter; // default to lambert for unimplemented brdfs

		funcs[muu::unwrap(material_type::metal)] = metal_scatter;

		return funcs;
	}();

	[[nodiscard]]
	static vec3 MUU_VECTORCALL trace(const rt::scene& scene, const ray r, unsigned max_bounces) noexcept
	{
		if (!(max_bounces--))
			return {};

		auto hit = test_planes(scene, r);
		hit		 = select(test_spheres(scene, r), hit);
		hit		 = select(test_boxes(scene, r), hit);
		if (!hit)
			return vec3::lerp(colours::white.rgb, vec3{ 0.5f, 0.7f, 1.0f }, 0.5f * (r.direction.y + 1.0f));

		vec3 attenuation;
		if (const auto scatter = scatter_funcs[static_cast<size_t>(scene.materials.type()[hit.material])](scene, //
																										  r,
																										  hit,
																										  attenuation))
			return attenuation * trace(scene, *scatter, max_bounces);

		return {};
	}

	struct mg_scalar_ray_tracer final : renderer_interface
	{
		void render(const rt::scene& scene, image_view& pixels, muu::thread_pool& threads) noexcept override
		{
			const auto view = scene.camera.viewport(pixels.size());

			const auto worker = [=, &scene](unsigned pixel_index) noexcept
			{
				const auto screen_pos = pixels.position_of(pixel_index);

				auto colour = vec3{};
				for (unsigned i = 0, e = scene.samples_per_pixel; i < e; i++)
				{
					const auto pos	= vec2{ screen_pos } + (i ? random<vec2>() : vec2{ 0.5f });
					const auto near = view.screen_to_world(pos, 0.0f);
					const auto far	= view.screen_to_world(pos, 1.0f);

					colour += trace(scene, ray{ near, vec3::direction(near, far) }, scene.max_bounces);
				}
				colour /= static_cast<float>(scene.samples_per_pixel);
				colour.x = std::sqrt(colour.x);
				colour.y = std::sqrt(colour.y);
				colour.z = std::sqrt(colour.z);

				pixels(screen_pos) = rt::colour{ colour };
			};

			threads.for_range(unsigned{}, pixels.size().x * pixels.size().y, worker);
			threads.wait();
		}
	};

	REGISTER_RENDERER(mg_scalar_ray_tracer);
}
