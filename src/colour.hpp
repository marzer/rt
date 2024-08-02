#pragma once
#include "common.hpp"

MUU_PUSH_WARNINGS;
#ifdef _MSC_VER
	#pragma warning(disable : 4201)
#elif MUU_GCC
	#pragma GCC diagnostic ignored "-Wpedantic"
	#pragma GCC diagnostic ignored "-Wshadow"
#elif MUU_CLANG
	#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
	#pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

namespace rt
{
	struct MUU_TRIVIAL_ABI colour
	{
		struct rgba_t
		{
			float r;
			float g;
			float b;
			float a;
		};
		static_assert(sizeof(rgba_t) == sizeof(float) * 4);

		struct rgb_t
		{
			float r;
			float g;
			float b;
		};
		static_assert(sizeof(rgb_t) == sizeof(float) * 3);

		union
		{
			rgba_t rgba;

			struct
			{
				union
				{
					rgb_t rgb;

					struct
					{
						float r;
						float g;
						float b;
					};
				};
				float a;
			};

			float values[4];
		};

		MUU_NODISCARD_CTOR
		colour() noexcept = default;

		MUU_NODISCARD_CTOR
		explicit constexpr colour(const vec3 rgb, float a = 1.0f) noexcept //
			: rgba{ muu::bit_cast<rgba_t>(vec4{ rgb, a }) }
		{}

		MUU_NODISCARD_CTOR
		explicit constexpr colour(const vec4 rgba) noexcept //
			: rgba{ muu::bit_cast<rgba_t>(rgba) }
		{}

		template <typename T>
		MUU_PURE_INLINE_GETTER
		static constexpr float MUU_VECTORCALL to_component_value(T val) noexcept
		{
			if constexpr (muu::is_floating_point<T>)
			{
				return muu::clamp(static_cast<float>(val), 0.0f, 1.0f);
			}
			else
			{
				return to_component_value(static_cast<float>(static_cast<muu::remove_enum<T>>(val)));
			}
		}

		template <typename R, typename G, typename B, typename A = float>
		MUU_NODISCARD_CTOR
		constexpr colour(R r, G g, B b, A a = 1.0f) noexcept //
			: rgba{ to_component_value(r), to_component_value(g), to_component_value(b), to_component_value(a) }
		{}

		MUU_NODISCARD_CTOR
		explicit constexpr colour(uint32_t rgba) noexcept
			: rgba{ to_component_value((rgba >> 24) & 0xFF),
					to_component_value((rgba >> 16) & 0xFF),
					to_component_value((rgba >> 8) & 0xFF),
					to_component_value(rgba & 0xFF) }
		{}

		MUU_PURE_GETTER
		/*implicit*/ MUU_VECTORCALL operator uint32_t() const noexcept
		{
			const auto src = vec4u{ vec4::clamp(muu::bit_cast<vec4>(rgba), vec4::constants::zero, vec4::constants::one)
									* vec4{ 255.99999f } };
			return (src.x << 24u) | (src.y << 16u) | (src.z << 8u) | src.w;
		}

		MUU_PURE_GETTER
		explicit constexpr MUU_VECTORCALL operator vec3() const noexcept
		{
			return muu::bit_cast<vec3>(rgb);
		}

		MUU_PURE_GETTER
		explicit constexpr MUU_VECTORCALL operator vec4() const noexcept
		{
			return muu::bit_cast<vec4>(rgba);
		}

	  private:
		MUU_ALWAYS_INLINE
		MUU_PURE
		friend constexpr colour& MUU_VECTORCALL operator+=(colour& lhs, colour rhs) noexcept
		{
			lhs.rgba = muu::bit_cast<rgba_t>(muu::bit_cast<vec4>(lhs.rgba) + muu::bit_cast<vec4>(rhs.rgba));
			return lhs;
		}

		MUU_PURE_INLINE_GETTER
		friend constexpr colour MUU_VECTORCALL operator+(colour lhs, colour rhs) noexcept
		{
			return lhs += rhs;
		}

		template <std::floating_point T>
		MUU_ALWAYS_INLINE
		MUU_PURE
		friend constexpr colour& MUU_VECTORCALL operator*=(colour& lhs, T rhs) noexcept
		{
			lhs.rgba = muu::bit_cast<rgba_t>(muu::bit_cast<vec4>(lhs.rgba) * rhs);
			return lhs;
		}

		template <std::floating_point T>
		MUU_PURE_INLINE_GETTER
		friend constexpr colour MUU_VECTORCALL operator*(colour lhs, T rhs) noexcept
		{
			return lhs *= rhs;
		}
	};
	static_assert(std::is_trivially_copyable_v<colour>);
	static_assert(sizeof(colour) == sizeof(vec4));

	inline namespace literals
	{
		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		colour operator""_rgba(unsigned long long rgba) noexcept
		{
			return colour{ static_cast<uint32_t>(rgba & 0xFFFFFFFFull) };
		}

		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		colour operator""_rgb(unsigned long long rgba) noexcept
		{
			return colour{ static_cast<uint32_t>(((rgba & 0xFFFFFFFFull) << 8) | 0x000000FFull) };
		}

		MUU_PURE_INLINE_GETTER
		MUU_CONSTEVAL
		colour operator""_argb(unsigned long long argb) noexcept
		{
			return colour{ static_cast<uint32_t>(((argb >> 24) & 0xFFull) | ((argb << 8) & 0xFFFFFF00ull)) };
		}
	}

	namespace colours
	{
		// named colours - https://htmlcolorcodes.com/color-names/
		inline constexpr colour alice_blue			   = 0xF0F8FF_rgb;
		inline constexpr colour antique_white		   = 0xFAEBD7_rgb;
		inline constexpr colour aqua				   = 0x00FFFF_rgb;
		inline constexpr colour aquamarine			   = 0x7FFFD4_rgb;
		inline constexpr colour azure				   = 0xF0FFFF_rgb;
		inline constexpr colour beige				   = 0xF5F5DC_rgb;
		inline constexpr colour bisque				   = 0xFFE4C4_rgb;
		inline constexpr colour black				   = 0x000000_rgb;
		inline constexpr colour blanched_almond		   = 0xFFEBCD_rgb;
		inline constexpr colour blue				   = 0x0000FF_rgb;
		inline constexpr colour blue_violet			   = 0x8A2BE2_rgb;
		inline constexpr colour brown				   = 0xA52A2A_rgb;
		inline constexpr colour burly_wood			   = 0xDEB887_rgb;
		inline constexpr colour cadet_blue			   = 0x5F9EA0_rgb;
		inline constexpr colour chartreuse			   = 0x7FFF00_rgb;
		inline constexpr colour chocolate			   = 0xD2691E_rgb;
		inline constexpr colour coral				   = 0xFF7F50_rgb;
		inline constexpr colour cornflower_blue		   = 0x6495ED_rgb;
		inline constexpr colour cornsilk			   = 0xFFF8DC_rgb;
		inline constexpr colour crimson				   = 0xDC143C_rgb;
		inline constexpr colour cyan				   = 0x00FFFF_rgb;
		inline constexpr colour dark_blue			   = 0x00008B_rgb;
		inline constexpr colour dark_cyan			   = 0x008B8B_rgb;
		inline constexpr colour dark_goldenrod		   = 0xB8860B_rgb;
		inline constexpr colour dark_gray			   = 0xA9A9A9_rgb;
		inline constexpr colour dark_green			   = 0x006400_rgb;
		inline constexpr colour dark_khaki			   = 0xBDB76B_rgb;
		inline constexpr colour dark_magenta		   = 0x8B008B_rgb;
		inline constexpr colour dark_olive_green	   = 0x556B2F_rgb;
		inline constexpr colour dark_orange			   = 0xFF8C00_rgb;
		inline constexpr colour dark_orchid			   = 0x9932CC_rgb;
		inline constexpr colour dark_red			   = 0x8B0000_rgb;
		inline constexpr colour dark_salmon			   = 0xE9967A_rgb;
		inline constexpr colour dark_sea_green		   = 0x8FBC8B_rgb;
		inline constexpr colour dark_slate_blue		   = 0x483D8B_rgb;
		inline constexpr colour dark_slate_gray		   = 0x2F4F4F_rgb;
		inline constexpr colour dark_turquoise		   = 0x00CED1_rgb;
		inline constexpr colour dark_violet			   = 0x9400D3_rgb;
		inline constexpr colour deep_pink			   = 0xFF1493_rgb;
		inline constexpr colour deep_sky_blue		   = 0x00BFFF_rgb;
		inline constexpr colour dim_gray			   = 0x696969_rgb;
		inline constexpr colour dodger_blue			   = 0x1E90FF_rgb;
		inline constexpr colour fire_brick			   = 0xB22222_rgb;
		inline constexpr colour floral_white		   = 0xFFFAF0_rgb;
		inline constexpr colour forest_green		   = 0x228B22_rgb;
		inline constexpr colour fuchsia				   = 0xFF00FF_rgb;
		inline constexpr colour gainsboro			   = 0xDCDCDC_rgb;
		inline constexpr colour ghost_white			   = 0xF8F8FF_rgb;
		inline constexpr colour gold				   = 0xFFD700_rgb;
		inline constexpr colour goldenrod			   = 0xDAA520_rgb;
		inline constexpr colour gray				   = 0x808080_rgb;
		inline constexpr colour green				   = 0x008000_rgb;
		inline constexpr colour green_yellow		   = 0xADFF2F_rgb;
		inline constexpr colour honey_dew			   = 0xF0FFF0_rgb;
		inline constexpr colour hot_pink			   = 0xFF69B4_rgb;
		inline constexpr colour indian_red			   = 0xCD5C5C_rgb;
		inline constexpr colour indigo				   = 0x4B0082_rgb;
		inline constexpr colour ivory				   = 0xFFFFF0_rgb;
		inline constexpr colour khaki				   = 0xF0E68C_rgb;
		inline constexpr colour lavender			   = 0xE6E6FA_rgb;
		inline constexpr colour lavender_blush		   = 0xFFF0F5_rgb;
		inline constexpr colour lawn_green			   = 0x7CFC00_rgb;
		inline constexpr colour lemon_chiffon		   = 0xFFFACD_rgb;
		inline constexpr colour light_blue			   = 0xADD8E6_rgb;
		inline constexpr colour light_coral			   = 0xF08080_rgb;
		inline constexpr colour light_cyan			   = 0xE0FFFF_rgb;
		inline constexpr colour light_goldenrod_yellow = 0xFAFAD2_rgb;
		inline constexpr colour light_gray			   = 0xD3D3D3_rgb;
		inline constexpr colour light_green			   = 0x90EE90_rgb;
		inline constexpr colour light_pink			   = 0xFFB6C1_rgb;
		inline constexpr colour light_salmon		   = 0xFFA07A_rgb;
		inline constexpr colour light_sea_green		   = 0x20B2AA_rgb;
		inline constexpr colour light_sky_blue		   = 0x87CEFA_rgb;
		inline constexpr colour light_slate_gray	   = 0x778899_rgb;
		inline constexpr colour light_steel_blue	   = 0xB0C4DE_rgb;
		inline constexpr colour light_yellow		   = 0xFFFFE0_rgb;
		inline constexpr colour lime				   = 0x00FF00_rgb;
		inline constexpr colour lime_green			   = 0x32CD32_rgb;
		inline constexpr colour linen				   = 0xFAF0E6_rgb;
		inline constexpr colour magenta				   = 0xFF00FF_rgb;
		inline constexpr colour maroon				   = 0x800000_rgb;
		inline constexpr colour medium_aquamarine	   = 0x66CDAA_rgb;
		inline constexpr colour medium_blue			   = 0x0000CD_rgb;
		inline constexpr colour medium_orchid		   = 0xBA55D3_rgb;
		inline constexpr colour medium_purple		   = 0x9370DB_rgb;
		inline constexpr colour medium_sea_green	   = 0x3CB371_rgb;
		inline constexpr colour medium_slate_blue	   = 0x7B68EE_rgb;
		inline constexpr colour medium_spring_green	   = 0x00FA9A_rgb;
		inline constexpr colour medium_turquoise	   = 0x48D1CC_rgb;
		inline constexpr colour medium_violet_red	   = 0xC71585_rgb;
		inline constexpr colour midnight_blue		   = 0x191970_rgb;
		inline constexpr colour mint_cream			   = 0xF5FFFA_rgb;
		inline constexpr colour misty_rose			   = 0xFFE4E1_rgb;
		inline constexpr colour moccasin			   = 0xFFE4B5_rgb;
		inline constexpr colour navajo_white		   = 0xFFDEAD_rgb;
		inline constexpr colour navy				   = 0x000080_rgb;
		inline constexpr colour old_lace			   = 0xFDF5E6_rgb;
		inline constexpr colour olive				   = 0x808000_rgb;
		inline constexpr colour olive_drab			   = 0x6B8E23_rgb;
		inline constexpr colour orange				   = 0xFFA500_rgb;
		inline constexpr colour orange_red			   = 0xFF4500_rgb;
		inline constexpr colour orchid				   = 0xDA70D6_rgb;
		inline constexpr colour pale_goldenrod		   = 0xEEE8AA_rgb;
		inline constexpr colour pale_green			   = 0x98FB98_rgb;
		inline constexpr colour pale_turquoise		   = 0xAFEEEE_rgb;
		inline constexpr colour pale_violet_red		   = 0xDB7093_rgb;
		inline constexpr colour papaya_whip			   = 0xFFEFD5_rgb;
		inline constexpr colour peach_puff			   = 0xFFDAB9_rgb;
		inline constexpr colour peru				   = 0xCD853F_rgb;
		inline constexpr colour pink				   = 0xFFC0CB_rgb;
		inline constexpr colour plum				   = 0xDDA0DD_rgb;
		inline constexpr colour powder_blue			   = 0xB0E0E6_rgb;
		inline constexpr colour purple				   = 0x800080_rgb;
		inline constexpr colour rebecca_purple		   = 0x663399_rgb;
		inline constexpr colour red					   = 0xFF0000_rgb;
		inline constexpr colour rosy_brown			   = 0xBC8F8F_rgb;
		inline constexpr colour royal_blue			   = 0x4169E1_rgb;
		inline constexpr colour saddle_brown		   = 0x8B4513_rgb;
		inline constexpr colour salmon				   = 0xFA8072_rgb;
		inline constexpr colour sandy_brown			   = 0xF4A460_rgb;
		inline constexpr colour sea_green			   = 0x2E8B57_rgb;
		inline constexpr colour sea_shell			   = 0xFFF5EE_rgb;
		inline constexpr colour sienna				   = 0xA0522D_rgb;
		inline constexpr colour silver				   = 0xC0C0C0_rgb;
		inline constexpr colour sky_blue			   = 0x87CEEB_rgb;
		inline constexpr colour slate_blue			   = 0x6A5ACD_rgb;
		inline constexpr colour slate_gray			   = 0x708090_rgb;
		inline constexpr colour snow				   = 0xFFFAFA_rgb;
		inline constexpr colour spring_green		   = 0x00FF7F_rgb;
		inline constexpr colour steel_blue			   = 0x4682B4_rgb;
		inline constexpr colour tan					   = 0xD2B48C_rgb;
		inline constexpr colour teal				   = 0x008080_rgb;
		inline constexpr colour thistle				   = 0xD8BFD8_rgb;
		inline constexpr colour tomato				   = 0xFF6347_rgb;
		inline constexpr colour turquoise			   = 0x40E0D0_rgb;
		inline constexpr colour violet				   = 0xEE82EE_rgb;
		inline constexpr colour wheat				   = 0xF5DEB3_rgb;
		inline constexpr colour white				   = 0xFFFFFF_rgb;
		inline constexpr colour white_smoke			   = 0xF5F5F5_rgb;
		inline constexpr colour yellow				   = 0xFFFF00_rgb;
		inline constexpr colour yellow_green		   = 0x9ACD32_rgb;

		// gray-dient
		inline constexpr colour gray_87 = 0x202020_rgb; // 12.5% white
		inline constexpr colour gray_75 = 0x404040_rgb; // 25% white
		inline constexpr colour gray_67 = 0x555555_rgb; // 33% white
		inline constexpr colour gray_50 = 0x808080_rgb; // 50% white
		inline constexpr colour gray_33 = 0xAAAAAA_rgb; // 67% white
		inline constexpr colour gray_25 = 0xC0C0C0_rgb; // 75% white

		// funsies
		inline constexpr colour portal_blue	  = 0x0078FF_rgb;
		inline constexpr colour portal_orange = 0xFD6600_rgb;
	}
}

namespace muu
{
	template <>
	inline constexpr bool allow_implicit_bit_cast<rt::colour, rt::vec4> = true;

	template <>
	inline constexpr bool allow_implicit_bit_cast<rt::colour::rgba_t, rt::vec4> = true;

	template <>
	inline constexpr bool allow_implicit_bit_cast<rt::colour::rgb_t, rt::vec3> = true;
}

MUU_POP_WARNINGS;
