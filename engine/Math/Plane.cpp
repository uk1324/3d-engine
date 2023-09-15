#include "Plane.hpp"

Plane::Plane(Vec3 n, float d)
	: n(n)
	, d(d) {}

Plane Plane::fromPoints(const Vec3& a, const Vec3& b, const Vec3& c) {
	const auto edge0 = c - a;
	const auto edge1 = b - a;
	const auto normal = cross(edge0, edge1).normalized();
	/*
	The shortest vector from a point to a plane is always perpendicular to the plane.
	If you draw a line segment perpendicular a line in 2D from any point then if you draw any other line to a
	different point on the plane then the line will create a hypotenuse of a triangle.
	The hypotenuse is always longer than the other edges.
	Projecting any point on the line onto the normalized normal will give the distance to the origin.
	*/
	const auto distance = dot(a, normal);
	return Plane(normal, distance);
}

float Plane::signedDistance(const Vec3& p) const {
	return d - dot(p, n);
}

bool Plane::isOnPositiveSide(const Vec3& p) const {
	return signedDistance(p) > 0;
}