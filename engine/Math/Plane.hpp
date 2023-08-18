#pragma once

#include "Vec3.hpp"

struct Plane {
	Plane(Vec3 n, float d);

	// dot(n, p) = d
	// Unit length
	Vec3 n;
	float d;
};