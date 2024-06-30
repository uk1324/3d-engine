#include "Circle.hpp"

bool circleContains(Vec2 circleCenter, float circleRadius, Vec2 position) {
	return (circleCenter - position).lengthSq() <= circleRadius * circleRadius;
}

bool circleAabbCollision(Vec2 circleCenter, float circleRadius, const Aabb& aabb) {
    if (aabb.contains(circleCenter)) {
        return true;
    } else {
        Vec2 closestPointToCircleCenterOnAabb = Vec2(
            std::clamp(circleCenter.x, aabb.min.x, aabb.max.x),
            std::clamp(circleCenter.y, aabb.min.y, aabb.max.y));
        return circleContains(circleCenter, circleRadius, closestPointToCircleCenterOnAabb);
    }
}