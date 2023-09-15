#include "Frustum.hpp"

Frustum Frustum::fromMatrix(const Mat4& toNdc) {
	const auto c = corners(toNdc);
	return Frustum{
		.planes = {
			Plane::fromPoints(c[FRONT_BOTTOM_LEFT_CORNER], c[FRONT_BOTTOM_RIGHT_CORNER], c[FRONT_TOP_RIGHT_CORNER]),
			Plane::fromPoints(c[BACK_TOP_RIGHT_CORNER], c[BACK_BOTTOM_RIGHT_CORNER], c[BACK_BOTTOM_LEFT_CORNER]),
			Plane::fromPoints(c[BACK_BOTTOM_LEFT_CORNER], c[FRONT_BOTTOM_LEFT_CORNER], c[FRONT_TOP_LEFT_CORNER]),
			Plane::fromPoints(c[FRONT_TOP_RIGHT_CORNER], c[FRONT_BOTTOM_RIGHT_CORNER], c[BACK_BOTTOM_RIGHT_CORNER]),
			Plane::fromPoints(c[FRONT_TOP_LEFT_CORNER], c[FRONT_TOP_RIGHT_CORNER], c[BACK_TOP_RIGHT_CORNER]),
			Plane::fromPoints(c[BACK_BOTTOM_RIGHT_CORNER], c[FRONT_BOTTOM_RIGHT_CORNER], c[FRONT_BOTTOM_LEFT_CORNER]),
		}
	};
}

std::array<Vec3, 8> Frustum::corners(const Mat4& toNdc) {
	const auto fromNdcToFrustum = toNdc.inversed();
	auto transform = [&](Vec3 v) -> Vec3 {
		const auto a = Vec4(v, 1.0f) * fromNdcToFrustum;
		return a.xyz() / a.w;
	};
	const std::array<Vec3, 8> corners = {
		transform(Vec3(1, 1, 1)),
		transform(Vec3(1, -1, 1)),
		transform(Vec3(-1, 1, 1)),
		transform(Vec3(-1, -1, 1)),
		transform(Vec3(1, 1, -1)),
		transform(Vec3(1, -1, -1)),
		transform(Vec3(-1, 1, -1)),
		transform(Vec3(-1, -1, -1)),
	};
	return corners;
}

bool Frustum::intersects(const Aabb3& aabb) const {
	auto anyPointsOfAabbOnNegativeSide = [&](const Plane& plane) -> bool {
		const auto size = aabb.size();
		return plane.isOnPositiveSide(aabb.min)
			|| plane.isOnPositiveSide(aabb.max)
			|| plane.isOnPositiveSide(aabb.min + Vec3(size.x, 0, 0))
			|| plane.isOnPositiveSide(aabb.min + Vec3(0, size.y, 0))
			|| plane.isOnPositiveSide(aabb.min + Vec3(0, 0, size.z))
			|| plane.isOnPositiveSide(aabb.min + Vec3(0, size.y, size.z))
			|| plane.isOnPositiveSide(aabb.min + Vec3(size.x, 0, size.z))
			|| plane.isOnPositiveSide(aabb.min + Vec3(size.x, size.y, 0));
	};

	bool inside = true;
	for (i32 i = 0; i < PLANE_COUNT; i++) {
		inside &= anyPointsOfAabbOnNegativeSide(planes[i]);
	}
	return inside;
}
