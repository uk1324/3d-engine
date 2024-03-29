#pragma once

#include "Line.hpp"
#include "Aabb.hpp"
#include <array>

std::optional<Vec2> intersectLineSegments(Vec2 s0, Vec2 e0, Vec2 s1, Vec2 e1);

struct LineSegment {
	LineSegment(Line line, float minDistanceAlongLine, float maxDistanceAlongLine);
	LineSegment(Vec2 start, Vec2 end);

	auto closestPointTo(Vec2 p) const -> Vec2;
	auto distance(Vec2 p) const -> float;
	auto asBoxContains(float width, Vec2 p) const -> bool;
	auto asCapsuleContains(float thickness, Vec2 p) const -> bool;
	auto aabb() const -> Aabb;
	auto getCorners() const -> std::array<Vec2, 2>;
	// This is essentially just LineSegment vs LineSegment intersection
	auto raycastHit(Vec2 rayBegin, Vec2 rayEnd) const -> std::optional<Vec2>;
	auto intersection(const LineSegment& other) const -> std::optional<Vec2>;

	Line line;
	float minDistanceAlongLine;
	float maxDistanceAlongLine;
};

inline LineSegment::LineSegment(Line line, float minDistanceAlongLine, float maxDistanceAlongLine)
	: line{ line }
	, minDistanceAlongLine{ minDistanceAlongLine }
	, maxDistanceAlongLine{ maxDistanceAlongLine }
{}

inline LineSegment::LineSegment(Vec2 start, Vec2 end)
	: line{ start, end } {
	float a = line.distanceAlong(start);
	float b = line.distanceAlong(end);
	if (a < b) {
		minDistanceAlongLine = a;
		maxDistanceAlongLine = b;
	} else {
		minDistanceAlongLine = b;
		maxDistanceAlongLine = a;
	}
}

auto inline LineSegment::intersection(const LineSegment& other) const -> std::optional<Vec2> {
	const auto intersection = line.intersection(other.line);
	if (!intersection.has_value())
		return std::nullopt;

	const auto distanceAlong = line.distanceAlong(*intersection);
	if (distanceAlong < minDistanceAlongLine || distanceAlong > maxDistanceAlongLine)
		return std::nullopt;

	const auto otherDistanceAlong = other.line.distanceAlong(*intersection);
	if (otherDistanceAlong < other.minDistanceAlongLine || otherDistanceAlong > other.maxDistanceAlongLine)
		return std::nullopt;

	return intersection;
}