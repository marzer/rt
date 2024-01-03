#include "../scene.hpp"
#include "../image.hpp"
#include "../renderer.hpp"
MUU_DISABLE_WARNINGS;
#include <muu/thread_pool.h>
MUU_ENABLE_WARNINGS;

using namespace rt;
using namespace muu::literals;

namespace
{
	MUU_PURE_GETTER
	static constexpr colour MUU_VECTORCALL lambert(vec3 surface_normal,
												   vec3 direction_to_light_source,
												   colour surface_color,
												   float intensity = 1.0f) noexcept
	{
		return colour{ direction_to_light_source.dot(surface_normal) * vec3{ surface_color } * intensity };
	}

	struct rasterizer final : renderer_interface
	{
		void render(const rt::scene& scene, image_view& pixels, muu::thread_pool& threads) noexcept override
		{
			const auto view = scene.camera.viewport(pixels.size());

			const auto worker = [=, &scene](unsigned pixel_index) noexcept
			{
				const auto screen_pos = pixels.position_of(pixel_index);
				const auto near_pos	  = view.screen_to_world(vec2{ screen_pos } + vec2{ 0.5f }, 0.0f);
				const auto far_pos	  = view.screen_to_world(vec2{ screen_pos } + vec2{ 0.5f }, 1.0f);
				const auto max_dist	  = vec3::distance(near_pos, far_pos);

				float dist		  = max_dist + 1.0f;
				auto hit_material = static_cast<unsigned>(-1);
				vec3 hit_pos	  = {};
				vec3 hit_normal	  = vec3::constants::up;
				const auto r	  = ray(near_pos, vec3::direction(near_pos, far_pos));

				const auto hit_tests = [&](auto& soa) noexcept
				{
					for (const auto& obj : std::span{ soa.value(), soa.size() })
					{
						using obj_type = std::remove_cvref_t<decltype(obj)>;

						const auto hit = r.hits(obj);
						if (!hit || *hit >= dist)
							continue;

						dist		 = *hit;
						hit_material = soa.material()[&obj - soa.value()];
						hit_pos		 = r.at(hit);

						if constexpr (std::is_same_v<obj_type, rt::sphere>)
							hit_normal = vec3::direction(obj.center, hit_pos);
						else if constexpr (std::is_same_v<obj_type, rt::plane>)
							hit_normal = obj.normal;
					}
				};
				hit_tests(scene.planes);
				hit_tests(scene.boxes);
				hit_tests(scene.spheres);

				static constexpr auto sky_end	= colour{ 238, 245, 255 };
				static constexpr auto sky_start = colour{ 208, 228, 255 };

				if (hit_material != static_cast<unsigned>(-1))
				{
					pixels(screen_pos) = colour{ vec3::min(vec3{ 0.25f }
															   + lambert(hit_normal,
																		 vec3::direction(hit_pos, near_pos),
																		 scene.materials.albedo()[hit_material])
																		 .rgb
																	 * vec3{ 0.75f },
														   vec3::constants::one) };
				}
				else
					pixels(screen_pos) = colour{ vec3::lerp(sky_start.rgb,
															sky_end.rgb,
															static_cast<float>(screen_pos.y)
																/ static_cast<float>(pixels.size().y - 1u)) };
			};

			threads.for_range(unsigned{}, pixels.size().x * pixels.size().y, worker);
			threads.wait();
		}
	};

	REGISTER_RENDERER(rasterizer);
}
