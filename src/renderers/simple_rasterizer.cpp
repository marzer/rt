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
	static const auto sky_end	= colour{ 238, 245, 255 };
	static const auto sky_start = colour{ 208, 228, 255 };

	struct simple_rasterizer final : renderer_interface
	{
		void MUU_VECTORCALL render(const rt::scene& scene,
								   image_view& pixels,
								   muu::thread_pool& threads) noexcept override
		{
			const auto view = scene.camera.viewport(pixels.size());

			const auto worker = [=, &scene](unsigned pixel_index) noexcept
			{
				const auto screen_pos = pixels.position_of(pixel_index);
				const auto near_pos	  = view.screen_to_world(vec2{ screen_pos } + vec2{ 0.5f }, 0.0f);
				const auto far_pos	  = view.screen_to_world(vec2{ screen_pos } + vec2{ 0.5f }, 1.0f);
				const auto max_dist	  = vec3::distance(near_pos, far_pos);

				float dist	  = max_dist + 1.0f;
				auto material = static_cast<unsigned>(-1);
				const auto r  = ray(near_pos, vec3::direction(near_pos, far_pos));

				const auto hit_tests = [&](auto& soa) noexcept
				{
					for (const auto& obj : std::span{ soa.value(), soa.size() })
					{
						const auto hit = r.hits(obj);
						if (!hit || *hit >= dist)
							continue;

						dist	 = *hit;
						material = soa.material()[&obj - soa.value()];
					}
				};
				hit_tests(scene.planes);
				hit_tests(scene.boxes);
				hit_tests(scene.spheres);

				pixels(screen_pos) =
					static_cast<uint32_t>(material != static_cast<unsigned>(-1)
											  ? scene.materials.colour()[material]
											  : colour{ vec3::lerp(sky_start.rgb,
																   sky_end.rgb,
																   static_cast<float>(screen_pos.y)
																	   / static_cast<float>(pixels.size().y - 1u)) });
			};

			threads.for_range(unsigned{}, pixels.size().x * pixels.size().y, worker);
			threads.wait();
		}
	};

	REGISTER_RENDERER(simple_rasterizer);
}
