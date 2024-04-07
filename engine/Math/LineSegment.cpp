#include "LineSegment.hpp"

auto LineSegment::closestPointTo(Vec2 p) const -> Vec2 {
	const auto along = line.distanceAlong(p);
	const auto alongClamped = std::clamp(along, minDistanceAlongLine, maxDistanceAlongLine);
	return line.n * line.d - line.n.rotBy90deg() * alongClamped;
}

auto LineSegment::distance(Vec2 p) const -> float {
	const auto along = line.distanceAlong(p);
	if (along < minDistanceAlongLine || along > maxDistanceAlongLine) {
		const auto alongClamped = std::clamp(along, minDistanceAlongLine, maxDistanceAlongLine);
		const auto lineEdge = line.n.rotBy90deg() * alongClamped;
		return ::distance(lineEdge, p);
	}
	return ::distance(line, p);
}

auto LineSegment::asBoxContains(float halfWidth, Vec2 p) const -> bool {
	const auto along = line.distanceAlong(p);
	if (along < minDistanceAlongLine || along > maxDistanceAlongLine)
		return false;

	return ::distance(line, p) < halfWidth;
}

auto LineSegment::asCapsuleContains(float thickness, Vec2 p) const -> bool {
	return distance(p) < thickness;
}

auto LineSegment::aabb() const -> Aabb {
	const auto corners = getCorners();
	return Aabb::fromCorners(corners[0], corners[1]);
}

auto LineSegment::getCorners() const -> std::array<Vec2, 2> {
	const auto alongLine = -line.n.rotBy90deg();
	const auto offset = line.n * line.d;
	return { alongLine * minDistanceAlongLine + offset, alongLine * maxDistanceAlongLine + offset };
}

auto LineSegment::raycastHit(Vec2 rayBegin, Vec2 rayEnd) const -> std::optional<Vec2> {
	const LineSegment rayLineSegment{ rayBegin, rayEnd };
	return intersection(rayLineSegment);
}

std::optional<Vec2> intersectLineSegments(Vec2 s0, Vec2 e0, Vec2 s1, Vec2 e1) {
	return LineSegment(s0, e0).intersection(LineSegment(s1, e1));
}
