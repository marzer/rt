#pragma once
#include "common.hpp"

namespace rt
{

	class light
	{
	  private:
		vec3 pos_ = { 0, 1, 0 };
		vec3 colour_ = { 0, 1, 0 };
		float intensity = 0.0;


	  public:
		MUU_PURE_INLINE_GETTER
		constexpr const vec3& position() const noexcept
		{
			return pos_;
		}

		// 	MUU_PURE_INLINE_GETTER
		// 	constexpr const mat3& rotation() const noexcept
		// 	{
		// 		return rot_;
		// 	}

		constexpr light& pose(const vec3& pos, const mat3& rot) noexcept
		{
			pos_ = pos;
			MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(pos_));

			// rot_ = muu::orthonormalize(rot);
			// MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(rot_));

			return *this;
		}

		constexpr light& pose(const vec3& pos, const vec3& dir) noexcept
		{
			return pose(pos, mat3::from_3d_direction(vec3::normalize(dir)));
		}
	};

};

// class camera
// {
//   private:
// 	float vfov_ = floats::pi_over_four;
// 	vec3 pos_	= { 0, 1, 0 };
// 	mat3 rot_	= mat3::constants::identity;
// 	float near_ = 0.01f;
// 	float far_	= 1000.0f;

//   public:
// 	MUU_NODISCARD_CTOR
// 	constexpr camera() noexcept = default;

// 	MUU_NODISCARD_CTOR
// 	constexpr camera(const camera&) noexcept = default;

// 	MUU_NODISCARD_CTOR
// 	explicit constexpr camera(const vec3& pos, const mat3& rot = mat3::constants::identity) noexcept //
// 		: pos_{ pos },
// 		  rot_{ mat3::orthonormalize(rot) }
// 	{
// 		MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(pos_));
// 		MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(rot_));
// 	}

// 	constexpr camera& operator=(const camera&) noexcept = default;

// 	~camera() noexcept = default;

// 	constexpr void rotate_yaw(float angle) noexcept
// 	{
// 		mat3 yaw_matrix = mat3::from_axis_angle(vec3::constants::up, angle);
// 		rot_			= yaw_matrix * rot_;
// 	}

// 	constexpr void rotate_pitch(float angle) noexcept
// 	{
// 		vec3 right		  = rot_.transform_direction(vec3::constants::right);
// 		mat3 pitch_matrix = mat3::from_axis_angle(right, angle);
// 		rot_			  = pitch_matrix * rot_;
// 	}

// 	MUU_PURE_INLINE_GETTER
// 	constexpr const vec3& position() const noexcept
// 	{
// 		return pos_;
// 	}

// 	MUU_PURE_INLINE_GETTER
// 	constexpr const mat3& rotation() const noexcept
// 	{
// 		return rot_;
// 	}

// 	constexpr camera& pose(const vec3& pos, const mat3& rot) noexcept
// 	{
// 		pos_ = pos;
// 		MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(pos_));

// 		rot_ = muu::orthonormalize(rot);
// 		MUU_CONSTEXPR_SAFE_ASSERT(!muu::infinity_or_nan(rot_));

// 		return *this;
// 	}

// 	constexpr camera& pose(const vec3& pos, const vec3& dir) noexcept
// 	{
// 		return pose(pos, mat3::from_3d_direction(vec3::normalize(dir)));
// 	}

// 	MUU_PURE_GETTER
// 	constexpr rt::viewport viewport(vec2u size) const noexcept
// 	{
// 		const auto dir = rot_.transform_direction(vec3::constants::forward);

// 		rt::viewport vp{ .position	 = pos_, //
// 						 .size		 = size,
// 						 .near_clip	 = { near_, plane{ pos_ + dir * near_, dir } },
// 						 .far_clip	 = { far_, plane{ pos_ + dir * far_, -dir } },
// 						 .view		 = mat4::invert(mat4::from_translation(pos_) * mat4::from_3d_rotation(rot_)),
// 						 .projection = mat4::perspective_projection(vfov_, vec2{ size }, near_, far_) };

// 		vp.view_projection		   = vp.projection * vp.view;
// 		vp.inverse_view_projection = mat4::invert(vp.view_projection);

// 		return vp;
// 	}
// };
// }
