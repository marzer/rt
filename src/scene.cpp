#include "scene.hpp"
MUU_DISABLE_WARNINGS;
#include <toml++/toml.h>
#include <iostream>
#include <stdexcept>
MUU_ENABLE_WARNINGS;

using namespace rt;

scene scene::load(std::string_view path)
{
	if (path.empty())
		throw std::runtime_error{ "no scene file path provided" };

	scene s;

	[[maybe_unused]] auto config = path == "-"sv ? toml::parse(std::cin, "stdin"sv) : toml::parse_file(path);

	return s;
}
