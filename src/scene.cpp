#include "scene.hpp"
MUU_DISABLE_WARNINGS;
#include <toml++/toml.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <concepts>
#include <muu/type_name.h>
MUU_ENABLE_WARNINGS;

using namespace rt;

namespace
{
	template <typename T>
	inline constexpr bool is_array = false;
	template <typename T, size_t N>
	inline constexpr bool is_array<std::array<T, N>> = true;
	template <typename T, size_t N>
	inline constexpr bool is_array<T[N]> = true;

	template <typename T>
	inline constexpr bool is_optional = false;
	template <typename T>
	inline constexpr bool is_optional<std::optional<T>> = true;

	namespace concepts
	{
		template <typename T>
		concept array = is_array<T>;

		template <typename T>
		concept optional = is_optional<T>;

		template <typename T>
		concept arithmetic_or_bool = std::integral<T> || std::floating_point<T>;
	}

	template <typename... T>
	[[noreturn]]
	static void error(const toml::node& node, T&&... args)
	{
		std::ostringstream msg;
		((msg << static_cast<T&&>(args)), ...);
		throw toml::parse_error{ std::move(msg).str().c_str(), node.source() };
	}

	template <typename T>
	[[noreturn]]
	static void mismatch_error(const toml::node& node, const T&)
	{
		std::ostringstream msg;
		msg << "No mapping from TOML "sv;
		if (auto arr = node.as_array())
		{
			if (arr->is_homogeneous())
				msg << arr->type() << "<"sv << (*arr)[0].type() << ">["sv << arr->size() << "]"sv;
			else
				msg << arr->type() << "["sv << arr->size() << "]"sv;
		}
		else
			msg << node.type();
		msg << " to " << muu::type_name<std::remove_cvref_t<T>>;

		throw toml::parse_error{ std::move(msg).str().c_str(), node.source() };
	}

	template <concepts::arithmetic_or_bool T>
	static auto& deserialize(const toml::node& node, T& val)
	{
		auto opt = node.value<std::remove_cvref_t<T>>();
		if (!opt.has_value())
			mismatch_error(node, val);
		if constexpr (std::floating_point<T>)
		{
			if (muu::infinity_or_nan(*opt))
				error(node, "Infinities and NaNs are not allowed."sv);
		}
		val = *opt;
		return val;
	}

	template <typename T, size_t N>
	static auto& deserialize(const toml::node& node, muu::vector<T, N>& val)
	{
		bool broadcasted = false;
		node.visit(
			[&](auto& n) noexcept
			{
				if constexpr (toml::is_number<decltype(n)>)
				{
					val			= muu::vector<T, N>{ static_cast<T>(*n) };
					broadcasted = true;
				}
			});
		if (broadcasted)
			return val;

		auto arr = node.as_array();
		if (!arr || arr->size() > N)
			mismatch_error(node, val);

		for (size_t i = 0; i < arr->size(); i++)
			deserialize<T>((*arr)[i], val[i]);

		return val;
	}

	template <typename T, size_t R, size_t C>
	static auto& deserialize(const toml::node& node, muu::matrix<T, R, C>& val)
	{
		auto arr = node.as_array();
		if (!arr || arr->size() > R * C)
			mismatch_error(node, val);

		for (size_t r = 0, i = 0; r < R; r++)
			for (size_t c = 0; c < C && i < arr->size(); c++, i++)
				deserialize((*arr)[i], val(r, c));

		return val;
	}

	template <concepts::array T>
	static auto& deserialize(const toml::node& node, T& val)
	{
		auto arr = node.as_array();
		if (!arr || arr->size() > (sizeof(val) / sizeof(val[0])))
			mismatch_error(node, val);

		size_t i = 0;
		for (auto& elem : *arr)
			deserialize(elem, val[i++]);

		return val;
	}

	template <concepts::optional T>
	static auto& deserialize(const toml::node& node, T& val)
	{
		val.emplace();
		deserialize(node, *val);
		return val;
	}

	template <typename T>
	static T& deserialize_if(const toml::node* node, T& val)
	{
		if (node)
			deserialize(*node, val);
		return val;
	}

	template <typename T>
	static T deserialize_if(const toml::node* node, T&& val)
	{
		return deserialize_if(node, val);
	}

	const toml::node* get(const toml::node& parent, std::string_view key, bool required = false)
	{
		auto view = toml::node_view{ parent }[key];
		if (!view)
		{
			if (required)
				error(parent, "missing required key '", key, "'");
			return nullptr;
		}

		return view.node();
	}

	const toml::table* get_table(const toml::node& parent, std::string_view key, bool required = false)
	{
		auto node = get(parent, key, required);
		if (!node)
			return nullptr;

		auto tbl = node->as_table();
		if (!tbl)
			error(parent, "expected table at key '", key, "', got ", node->type());

		return tbl;
	}

	const toml::array* get_array(const toml::node& parent, std::string_view key, bool required = false)
	{
		auto node = get(parent, key, required);
		if (!node)
			return nullptr;

		auto arr = node->as_array();
		if (!arr)
			error(parent, "expected array at key '", key, "', got ", node->type());

		return arr;
	}

	template <typename T>
	static T& deserialize(const toml::node& parent, std::string_view key, T& val)
	{
		return deserialize_if(get(parent, key), val);
	}

	template <typename T>
	static T deserialize(const toml::node& parent, std::string_view key, T&& val)
	{
		return deserialize_if(get(parent, key), val);
	}

	template <typename T>
	static T deserialize(const toml::node& parent, std::string_view key, const T& val)
	{
		return deserialize_if(get(parent, key), T{ val });
	}
}

scene scene::load(std::string_view path)
{
	if (path.empty())
		throw std::runtime_error{ "no scene file path provided" };

	scene s;

	auto config = path == "-"sv ? toml::parse(std::cin, "stdin"sv) : toml::parse_file(path);

	s.samples_per_pixel = muu::clamp(deserialize(config, "samples_per_pixel", 10u), 1u, 1000u);

	if (auto camera = get_table(config, "camera"))
	{
		s.camera.pose(deserialize(*camera, "position", vec3{ 0, 1, 0 }),
					  deserialize(*camera, "direction", vec3::constants::forward));
	}

	if (auto planes = get_array(config, "planes"))
	{
		for (auto& vals : *planes)
		{
			const auto plane = rt::plane{ deserialize(vals, "position", vec3{ 0, 0, 0 }),
										  vec3::normalize(deserialize(vals, "normal", vec3{ 0, 1, 0 })) };

			s.planes.push_back(plane, plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
		}
	}

	if (auto spheres = get_array(config, "spheres"))
	{
		for (auto& vals : *spheres)
		{
			const auto sphere = rt::sphere{ deserialize(vals, "position", vec3{ 0, 1, -3 }), //
											deserialize(vals, "radius", 0.5f) };

			s.spheres.push_back(sphere, sphere.center.x, sphere.center.y, sphere.center.z, sphere.radius);
		}
	}

	if (auto boxes = get_array(config, "boxes"))
	{
		for (auto& vals : *boxes)
		{
			const auto pos	   = deserialize(vals, "position", vec3{ 0, 1, -3 });
			const auto extents = deserialize(vals, "extents", vec3{ 0.5f });

			s.boxes.push_back(pos.x, pos.y, pos.z, extents.x, extents.y, extents.z);
		}
	}

	return s;
}
