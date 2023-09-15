#pragma once

#include "Plane.hpp"
#include "Mat4.hpp"
#include "Aabb3.hpp"
#include <array>

struct Frustum {
	enum Corner {
		BACK_TOP_RIGHT_CORNER,
		BACK_BOTTOM_RIGHT_CORNER,
		BACK_TOP_LEFT_CORNER,
		BACK_BOTTOM_LEFT_CORNER,
		FRONT_TOP_RIGHT_CORNER,
		FRONT_BOTTOM_RIGHT_CORNER,
		FRONT_TOP_LEFT_CORNER,
		FRONT_BOTTOM_LEFT_CORNER,
		CORNER_COUNT,
	};

	enum PlaneType {
		FRONT_PLANE,
		BACK_PLANE,
		LEFT_PLANE,
		RIGHT_PLANE,
		UP_PLANE,
		DOWN_PLANE,
		PLANE_COUNT,
	};

	static Frustum fromMatrix(const Mat4& toNdc);
	static std::array<Vec3, CORNER_COUNT> corners(const Mat4& toNdc);

	// Being inside also counts as intersecting.
	bool intersects(const Aabb3& aabb) const;

	Plane planes[PLANE_COUNT];
};