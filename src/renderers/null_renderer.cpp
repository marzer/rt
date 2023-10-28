#include "../scene.hpp"
#include "../image.hpp"
#include "../renderer.hpp"

using namespace rt;

namespace
{
	struct null_renderer final : renderer_interface
	{
		void MUU_VECTORCALL render(const rt::scene& /*scene*/,
								   image_view& /*pixels*/,
								   muu::thread_pool& /*threads*/) noexcept override
		{
			//
		}
	};

	REGISTER_RENDERER(null_renderer);
}
