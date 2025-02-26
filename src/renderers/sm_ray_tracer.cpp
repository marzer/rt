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
		attenuation = vec3{ scene.materials.albedo()[hit.material] * scene.materials.reflectivity()[hit.material] };

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
		attenuation = vec3{ scene.materials.albedo()[hit.material] * scene.materials.reflectivity()[hit.material] };

		vec3 scatter = reflect(vec3::normalize(r.direction), hit.normal)
					 + scene.materials.roughness()[hit.material] * random_unit_vector();
		if (vec3::dot(scatter, hit.normal) <= 0.0f)
			return {};

		scatter.normalize();
		return ray{ r.at(hit.distance), scatter };
	}

	// TODO: impliment this, it is currently metal settings
	// Here's how you might modify the dielectric_scatter function:

	// Snell's Law: Calculate the refracted ray using Snell's law.
	// Reflectance: Use the Schlick approximation to determine the probability of reflection vs. refraction.
	// Attenuation: For dielectrics, this is typically just a constant (e.g., the color white) since they usually don't
	// absorb much light.
	// 	Refraction Calculation: The refract function calculates the refracted direction using Snell's law.
	// Schlick's Approximation: The schlick function approximates the reflectance probability based on the angle and
	// indices of refraction. Attenuation: Dielectrics typically don't change the color much, so it's set to white
	// (vec3{1.0f, 1.0f, 1.0f}). Random Choice: A random number generator is used to decide between reflection and
	// refraction based on the calculated probabilities. You might need to adapt the reflect, refract, and schlick
	// functions based on your existing implementations.

	vec3 reflect(const vec3& v, const vec3& n)
	{
		return v - 2 * vec3::dot(v, n) * n;
	}

	bool refract(const vec3& v, const vec3& n, float eta, vec3& refracted)
	{
		float cos_i	 = -vec3::dot(v, n);
		float sin2_t = eta * eta * (1 - cos_i * cos_i);
		if (sin2_t > 1.0f)
		{
			return false; // Total internal reflection
		}
		float cos_t = static_cast<float>(sqrt(1.0f - sin2_t));
		refracted	= eta * v + (eta * cos_i - cos_t) * n;
		return true;
	}

	float schlick(float cosine, float ref_idx)
	{
		float r0 = (1 - ref_idx) / (1 + ref_idx);
		r0		 = r0 * r0;
		return static_cast<float>(r0 + (1 - r0) * pow((1 - cosine), 5));
	}

	MUU_PURE_GETTER
	static std::optional<ray> MUU_VECTORCALL dielectric_scatter(const rt::scene& scene,
																const ray& r,
																const hit_result& hit,
																vec3& attenuation) noexcept
	{
		vec3 outward_normal;
		vec3 reflected = reflect(r.direction, hit.normal);
		float ni_over_nt;
		vec3 refracted;
		float reflect_prob;
		float cosine;

		attenuation = vec3{ scene.materials.albedo()[hit.material] * scene.materials.reflectivity()[hit.material] };

		if (vec3::dot(r.direction, hit.normal) > 0.0f)
		{
			outward_normal = -hit.normal;
			ni_over_nt	   = scene.materials.reflectivity()[hit.material];
			cosine		   = scene.materials.reflectivity()[hit.material] * vec3::dot(r.direction, hit.normal)
				   / r.direction.length();
		}
		else
		{
			outward_normal = hit.normal;
			ni_over_nt	   = 1.0f / scene.materials.reflectivity()[hit.material];
			cosine		   = -vec3::dot(r.direction, hit.normal) / r.direction.length();
		}

		if (refract(r.direction, outward_normal, ni_over_nt, refracted))
			reflect_prob = schlick(cosine, scene.materials.reflectivity()[hit.material]);
		else
			reflect_prob = 1.0f;

		if (rt::random<float>() < reflect_prob)
			return ray{ r.at(hit.distance), reflected };
		else
			return ray{ r.at(hit.distance), refracted };
	}

	static constexpr auto scatter_funcs = []() noexcept
	{
		std::array<scatter_func*, magic_enum::enum_count<material_type>()> funcs{};

		for (auto& func : funcs)
			func = lambert_scatter; // default to lambert for unimplemented brdfs
		// TODO: is there a smarter way to do this
		funcs[muu::unwrap(material_type::metal)]	  = metal_scatter;
		funcs[muu::unwrap(material_type::dielectric)] = dielectric_scatter;
		funcs[muu::unwrap(material_type::air)]		  = dielectric_scatter;
		funcs[muu::unwrap(material_type::vacuum)]	  = dielectric_scatter;
		funcs[muu::unwrap(material_type::water)]	  = dielectric_scatter;
		funcs[muu::unwrap(material_type::ice)]		  = dielectric_scatter;

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

	struct sm_ray_tracer final : renderer_interface
	{
		void render(const rt::scene& scene, image_view& pxls, muu::thread_pool& threads) noexcept override
		{
			const auto view	  = scene.camera.viewport(pxls.size());
			const auto worker = [=, &scene](unsigned pixel_index) noexcept
			{
				const auto screen_pos = pxls.position_of(pixel_index);

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

				pxls(screen_pos) = rt::colour{ colour };
			};

			threads.for_range(unsigned{}, pxls.size().x * pxls.size().y, worker);
			threads.wait();
		}
	};

	REGISTER_RENDERER(sm_ray_tracer);
}
