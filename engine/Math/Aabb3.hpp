#pragma once

#include "Vec3.hpp"

struct Aabb3 {
	// Assumes min < max.
	static Aabb3 fromMinMax(const Vec3& min, const Vec3& max);
	static Aabb3 fromCorners(const Vec3& a, const Vec3& b);
	
	bool contains(const Vec3& p) const;
	Vec3 size() const;
	std::array<Vec3, 8> corners() const;

	Vec3 min, max;
};