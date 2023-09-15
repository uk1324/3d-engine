#include "Aabb3.hpp"
#include <array>

Aabb3 Aabb3::fromMinMax(const Vec3& min, const Vec3& max) {
	return Aabb3{
		.min = min,
		.max = max
	};
}

Aabb3 Aabb3::fromCorners(const Vec3& a, const Vec3& b) {
	Vec3 min, max;
	if (a.x < b.x) {
		min.x = a.x;
		max.x = b.x;
	} else {
		min.x = b.x;
		max.x = a.x;
	}

	if (a.y < b.y) {
		min.y = a.y;
		max.y = b.y;
	} else {
		min.y = b.y;
		max.y = a.y;
	}

	if (a.z < b.z) {
		min.z = a.z;
		max.z = b.z;
	} else {
		min.z = b.z;
		max.z = a.z;
	}
	return Aabb3::fromMinMax(min, max);
}

bool Aabb3::contains(const Vec3& p) const {
	return (p.x > min.x) && (p.x < max.x)
		&& (p.y > min.y) && (p.y < max.y)
		&& (p.z > min.z) && (p.z < max.z);
}

Vec3 Aabb3::size() const {
	return max - min;
}

std::array<Vec3, 8> Aabb3::corners() const {
	return {
		// Front (min) face counter clockwise
		min,
		Vec3(max.x, min.y, min.z),
		Vec3(max.x, max.y, min.z),
		Vec3(min.x, max.y, min.z),
		// Back (max) face counter clockwise (when looking from back.
		max,
		Vec3(max.x, min.y, max.z),
		Vec3(min.x, min.y, max.z),
		Vec3(min.x, max.y, max.z),
	};
}
