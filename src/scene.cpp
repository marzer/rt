#include "scene.hpp"
MUU_DISABLE_WARNINGS;
#include <toml++/toml.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <concepts>
#include <muu/type_name.h>
#include <muu/hashing.h>
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
	MUU_PURE_GETTER
	static constexpr uint64_t fnv1a_hash(std::string_view str, const T&... strs) noexcept
	{
		muu::fnv1a<64> hasher;
		hasher(str);
		(hasher(strs), ...);
		return hasher.value();
	}

	template <typename... T>
	[[noreturn]]
	static void error(const toml::node& node, T&&... args)
	{
		std::ostringstream msg;
		((msg << static_cast<T&&>(args)), ...);
		msg << "\n\n" << node.source();
		throw std::runtime_error{ std::move(msg).str().c_str() };
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

		msg << "\n\n" << node.source();
		throw std::runtime_error{ std::move(msg).str().c_str() };
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

#define string_vector_case_fallthrough(name)                                                                           \
	case fnv1a_hash(#name): [[fallthrough]]
#define string_vector_case(name)                                                                                       \
	case fnv1a_hash(#name): return val = muu::vector<T, N>::constants::name

	template <typename T, size_t N>
	static auto& deserialize(const toml::node& node, muu::vector<T, N>& val)
	{
		if (const auto str = node.as_string())
		{
			switch (fnv1a_hash(**str))
			{
				string_vector_case_fallthrough(origin);
				string_vector_case(zero);
				string_vector_case(one);
				string_vector_case(forward);
				string_vector_case_fallthrough(back);
				string_vector_case(backward);
				string_vector_case(up);
				string_vector_case(down);
				string_vector_case(left);
				string_vector_case(right);
				string_vector_case_fallthrough(x);
				string_vector_case(x_axis);
				string_vector_case_fallthrough(y);
				string_vector_case(y_axis);
				string_vector_case_fallthrough(z);
				string_vector_case(z_axis);
			}

			error(node, "unknown vector alias '"sv, **str, "'"sv);
		}

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
			deserialize((*arr)[i], val[i]);

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

#define string_colour_case(name)                                                                                       \
	case fnv1a_hash(#name): return val = colours::name

	static auto& deserialize(const toml::node& node, colour& val)
	{
		if (const auto str = node.as_string())
		{
			switch (fnv1a_hash(**str))
			{
				string_colour_case(alice_blue);
				string_colour_case(antique_white);
				string_colour_case(aqua);
				string_colour_case(aquamarine);
				string_colour_case(azure);
				string_colour_case(beige);
				string_colour_case(bisque);
				string_colour_case(black);
				string_colour_case(blanched_almond);
				string_colour_case(blue);
				string_colour_case(blue_violet);
				string_colour_case(brown);
				string_colour_case(burly_wood);
				string_colour_case(cadet_blue);
				string_colour_case(chartreuse);
				string_colour_case(chocolate);
				string_colour_case(coral);
				string_colour_case(cornflower_blue);
				string_colour_case(cornsilk);
				string_colour_case(crimson);
				string_colour_case(cyan);
				string_colour_case(dark_blue);
				string_colour_case(dark_cyan);
				string_colour_case(dark_goldenrod);
				string_colour_case(dark_gray);
				string_colour_case(dark_green);
				string_colour_case(dark_khaki);
				string_colour_case(dark_magenta);
				string_colour_case(dark_olive_green);
				string_colour_case(dark_orange);
				string_colour_case(dark_orchid);
				string_colour_case(dark_red);
				string_colour_case(dark_salmon);
				string_colour_case(dark_sea_green);
				string_colour_case(dark_slate_blue);
				string_colour_case(dark_slate_gray);
				string_colour_case(dark_turquoise);
				string_colour_case(dark_violet);
				string_colour_case(deep_pink);
				string_colour_case(deep_sky_blue);
				string_colour_case(dim_gray);
				string_colour_case(dodger_blue);
				string_colour_case(fire_brick);
				string_colour_case(floral_white);
				string_colour_case(forest_green);
				string_colour_case(fuchsia);
				string_colour_case(gainsboro);
				string_colour_case(ghost_white);
				string_colour_case(gold);
				string_colour_case(goldenrod);
				string_colour_case(gray);
				string_colour_case(green);
				string_colour_case(green_yellow);
				string_colour_case(honey_dew);
				string_colour_case(hot_pink);
				string_colour_case(indian_red);
				string_colour_case(indigo);
				string_colour_case(ivory);
				string_colour_case(khaki);
				string_colour_case(lavender);
				string_colour_case(lavender_blush);
				string_colour_case(lawn_green);
				string_colour_case(lemon_chiffon);
				string_colour_case(light_blue);
				string_colour_case(light_coral);
				string_colour_case(light_cyan);
				string_colour_case(light_goldenrod_yellow);
				string_colour_case(light_gray);
				string_colour_case(light_green);
				string_colour_case(light_pink);
				string_colour_case(light_salmon);
				string_colour_case(light_sea_green);
				string_colour_case(light_sky_blue);
				string_colour_case(light_slate_gray);
				string_colour_case(light_steel_blue);
				string_colour_case(light_yellow);
				string_colour_case(lime);
				string_colour_case(lime_green);
				string_colour_case(linen);
				string_colour_case(magenta);
				string_colour_case(maroon);
				string_colour_case(medium_aquamarine);
				string_colour_case(medium_blue);
				string_colour_case(medium_orchid);
				string_colour_case(medium_purple);
				string_colour_case(medium_sea_green);
				string_colour_case(medium_slate_blue);
				string_colour_case(medium_spring_green);
				string_colour_case(medium_turquoise);
				string_colour_case(medium_violet_red);
				string_colour_case(midnight_blue);
				string_colour_case(mint_cream);
				string_colour_case(misty_rose);
				string_colour_case(moccasin);
				string_colour_case(navajo_white);
				string_colour_case(navy);
				string_colour_case(old_lace);
				string_colour_case(olive);
				string_colour_case(olive_drab);
				string_colour_case(orange);
				string_colour_case(orange_red);
				string_colour_case(orchid);
				string_colour_case(pale_goldenrod);
				string_colour_case(pale_green);
				string_colour_case(pale_turquoise);
				string_colour_case(pale_violet_red);
				string_colour_case(papaya_whip);
				string_colour_case(peach_puff);
				string_colour_case(peru);
				string_colour_case(pink);
				string_colour_case(plum);
				string_colour_case(powder_blue);
				string_colour_case(purple);
				string_colour_case(rebecca_purple);
				string_colour_case(red);
				string_colour_case(rosy_brown);
				string_colour_case(royal_blue);
				string_colour_case(saddle_brown);
				string_colour_case(salmon);
				string_colour_case(sandy_brown);
				string_colour_case(sea_green);
				string_colour_case(sea_shell);
				string_colour_case(sienna);
				string_colour_case(silver);
				string_colour_case(sky_blue);
				string_colour_case(slate_blue);
				string_colour_case(slate_gray);
				string_colour_case(snow);
				string_colour_case(spring_green);
				string_colour_case(steel_blue);
				string_colour_case(tan);
				string_colour_case(teal);
				string_colour_case(thistle);
				string_colour_case(tomato);
				string_colour_case(turquoise);
				string_colour_case(violet);
				string_colour_case(wheat);
				string_colour_case(white);
				string_colour_case(white_smoke);
				string_colour_case(yellow);
				string_colour_case(yellow_green);
				string_colour_case(gray_87);
				string_colour_case(gray_75);
				string_colour_case(gray_67);
				string_colour_case(gray_50);
				string_colour_case(gray_33);
				string_colour_case(gray_25);
				string_colour_case(portal_blue);
				string_colour_case(portal_orange);
			}

			error(node, "unknown colour alias '"sv, **str, "'"sv);
		}

		auto arr = node.as_array();
		if (!arr || arr->size() > 4)
			mismatch_error(node, val);

		val.xyzw = {};
		for (size_t i = 0; i < arr->size(); i++)
			deserialize((*arr)[i], val.values[i]);
		if (arr->size() < 4)
			val.a = 1.0f;
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

	s.samples_per_pixel = muu::clamp(deserialize(config, "samples_per_pixel", 30u), 1u, 1000u);
	s.max_bounces		= muu::clamp(deserialize(config, "max_bounces", 10u), 1u, 1000u);
	s.low_res_mode		= false;

	if (auto camera = get_table(config, "camera"))
	{
		s.camera.pose(deserialize(*camera, "position", vec3{ 0, 1, 0 }),
					  deserialize(*camera, "direction", vec3::constants::forward));
	}

	if (auto materials = get_array(config, "materials"))
	{
		for (auto& tbl : *materials)
		{
			s.materials.push_back(deserialize(tbl, "colour", colour{}));
		}
	}
	if (s.materials.empty())
		s.materials.push_back(0xFF0080_rgb);

	const auto get_material = [&](const toml::node& parent) -> size_t
	{
		const auto material = deserialize(parent, "material", size_t{});
		if (material >= s.materials.size())
			error(parent, "material index "sv, material, " out-of-range");
		return material;
	};

	if (auto planes = get_array(config, "planes"))
	{
		for (auto& tbl : *planes)
		{
			const auto plane = rt::plane{ deserialize(tbl, "position", vec3{ 0, 0, 0 }),
										  vec3::normalize(deserialize(tbl, "normal", vec3{ 0, 1, 0 })) };

			s.planes.push_back(plane, get_material(tbl), plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
		}
	}

	if (auto spheres = get_array(config, "spheres"))
	{
		for (auto& tbl : *spheres)
		{
			const auto sphere = rt::sphere{ deserialize(tbl, "position", vec3{ 0, 1, -3 }), //
											deserialize(tbl, "radius", 0.5f) };

			s.spheres
				.push_back(sphere, get_material(tbl), sphere.center.x, sphere.center.y, sphere.center.z, sphere.radius);
		}
	}

	if (auto boxes = get_array(config, "boxes"))
	{
		for (auto& tbl : *boxes)
		{
			const auto box = rt::box{ deserialize(tbl, "position", vec3{ 0, 1, -3 }), //
									  deserialize(tbl, "extents", vec3{ 0.5f }) };

			s.boxes.push_back(box,
							  get_material(tbl),
							  box.center.x,
							  box.center.y,
							  box.center.z,
							  box.extents.x,
							  box.extents.y,
							  box.extents.z);
		}
	}

	return s;
}
