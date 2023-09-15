#pragma once

#include "Vec3.hpp"

struct Plane {
	// Assumes that n is normalized.
	Plane(Vec3 n, float d);
	static Plane fromPoints(const Vec3& a, const Vec3& b, const Vec3& c);

	float signedDistance(const Vec3& p) const;
	// The positive is the one that the normal points to. (TODO: Check)
	bool isOnPositiveSide(const Vec3& p) const;

	// dot(n, p) = d
	Vec3 n; // Unit length
	float d;
};