#include "renderer.hpp"
MUU_DISABLE_WARNINGS;
#include <vector>
MUU_ENABLE_WARNINGS;

using namespace rt;
using namespace rt::renderers;

namespace
{
	static std::vector<description>& all_renderers() noexcept
	{
		static std::vector<description> r;
		return r;
	}
}

void renderers::install(const description& desc)
{
	assert(!desc.key.empty());
	assert(!desc.name.empty());
	assert(desc.create);

	for (auto& r : all_renderers())
	{
		if (r.key == desc.key)
		{
			r = desc;
			return;
		}
	}

	all_renderers().push_back(desc);
}

std::span<const description> renderers::all() noexcept
{
	if (all_renderers().empty())
		return {};

	return { all_renderers().data(), all_renderers().size() };
}

const description* renderers::find_by_key(std::string_view val) noexcept
{
	if (val.empty())
		return {};

	for (const auto& r : all_renderers())
		if (r.key == val)
			return &r;

	return {};
}

const description* renderers::find_by_name(std::string_view val) noexcept
{
	if (val.empty())
		return {};

	for (const auto& r : all_renderers())
		if (r.name == val)
			return &r;

	return {};
}
