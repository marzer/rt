#pragma once
#include "common.hpp"
MUU_DISABLE_WARNINGS;
#include <span>
MUU_ENABLE_WARNINGS;

namespace rt
{
	struct MUU_ABSTRACT_INTERFACE renderer_interface
	{
		virtual void render(const scene&, image_view&, muu::thread_pool&) noexcept = 0;

		virtual ~renderer_interface() noexcept = default;
	};

	namespace renderers
	{
		struct description
		{
			using create_func = renderer_interface*();

			std::string_view key;
			std::string_view name;
			create_func* create;
		};

		void install(const description& desc);
		std::span<const description> all() noexcept;
		const description* find_by_key(std::string_view) noexcept;
		const description* find_by_name(std::string_view) noexcept;
	}
}

#define REGISTER_RENDERER(T)                                                                                           \
	static const int MUU_CONCAT(register_val_impl_, __LINE__) =                                                        \
		(::rt::renderers::install({ .key	= __FILE__ ":" MUU_MAKE_STRING(__LINE__) ":" MUU_MAKE_STRING(T),           \
									.name	= MUU_MAKE_STRING(T),                                                      \
									.create = []() -> renderer_interface* { return new T; } }),                        \
		 0)
