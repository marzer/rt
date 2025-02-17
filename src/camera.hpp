#pragma once
#include "common.hpp"

namespace rt
{
	struct viewport
	{
		vec3 position;
		vec2u size;
		struct clip_plane
		{
			float distance;
			rt::plane plane;
		} near_clip, far_clip;
		mat4 view;
		mat4 projection;
		mat4 view_projection;
		mat4 inverse_view_projection;

		MUU_PURE_GETTER
		constexpr vec2 world_to_screen(const vec3& world_pos, float& depth_out) const noexcept
		{
			auto pos_in_view_space = view_projection * vec4{ world_pos, 1.0f };
			if (pos_in_view_space.w != 0.0f)
				pos_in_view_space /= pos_in_view_space.w;

			depth_out = pos_in_view_space.z;

			return { (pos_in_view_space.x + 1.0f) * (static_cast<float>(size.x) / 2.0f),
					 (1.0f - pos_in_view_space.y) * (static_cast<float>(size.y) / 2.0f) };
		}

		MUU_PURE_GETTER
		constexpr vec2 world_to_screen(const vec3& world_pos) const noexcept
		{
			[[maybe_unused]]
			float f;
			return world_to_screen(world_pos, f);
		}

		MUU_PURE_GETTER
		constexpr vec3 screen_to_world(const vec2& screen_pos, float depth = 0.0f) const noexcept
		{
			const vec3 pos_in_view_space = { 2.0f * (screen_pos.x / static_cast<float>(size.x)) - 1.0f,
											 -2.0f * (screen_pos.y / static_cast<float>(size.y)) + 1.0f,
											 depth };
			return inverse_view_projection.transform_position(pos_in_view_space);
		}
	};

	class camera
	{
	  private:
		float vfov_ = floats::pi_over_four;
		vec3 pos_	= { 0, 1, 0 };
		mat3 rot_	= mat3::constants::identity;
		float near_ = 0.01f;
		float far_	= 1000.0f;

	  public:
		MUU_NODISCARD_CTOR
		constexpr camera() noexcept = default;

		MUU_NODISCARD_CTOR
		constexpr camera(const camera&) noexcept = default;

		MUU_NODISCARD_CTOR
		explicit constexpr camera(const vec3& pos, const mat3& rot = mat3::constants::identity) noexcept //
			: pos_{ pos },
			  rot_{ mat3::orthonormalize(rot) }
		{
			MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(pos_));
			MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(rot_));
		}

		constexpr camera& operator=(const camera&) noexcept = default;

		~camera() noexcept = default;

		constexpr void yaw(float angle) noexcept
		{
			mat3 yaw_matrix = mat3::from_axis_angle(vec3::constants::up, angle);
			rot_			= yaw_matrix * rot_;
		}

		constexpr void pitch(float angle) noexcept
		{
			vec3 right		  = rot_.transform_direction(vec3::constants::right);
			mat3 pitch_matrix = mat3::from_axis_angle(right, angle);
			rot_			  = pitch_matrix * rot_;
		}

		MUU_PURE_INLINE_GETTER
		constexpr const vec3& position() const noexcept
		{
			return pos_;
		}

		MUU_PURE_INLINE_GETTER
		constexpr const mat3& rotation() const noexcept
		{
			return rot_;
		}

		constexpr camera& pose(const vec3& pos, const mat3& rot) noexcept
		{
			pos_ = pos;
			MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(pos_));

			rot_ = muu::orthonormalize(rot);
			MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(rot_));

			return *this;
		}

		constexpr camera& pose(const vec3& pos, const vec3& dir) noexcept
		{
			return pose(pos, mat3::from_3d_direction(vec3::normalize(dir)));
		}

		MUU_PURE_GETTER
		constexpr rt::viewport viewport(vec2u size) const noexcept
		{
			const auto dir = rot_.transform_direction(vec3::constants::forward);

			rt::viewport vp{ .position	 = pos_, //
							 .size		 = size,
							 .near_clip	 = { near_, plane{ pos_ + dir * near_, dir } },
							 .far_clip	 = { far_, plane{ pos_ + dir * far_, -dir } },
							 .view		 = mat4::invert(mat4::from_translation(pos_) * mat4::from_3d_rotation(rot_)),
							 .projection = mat4::perspective_projection(vfov_, vec2{ size }, near_, far_) };

			vp.view_projection		   = vp.projection * vp.view;
			vp.inverse_view_projection = mat4::invert(vp.view_projection);

			return vp;
		}
	};
}
