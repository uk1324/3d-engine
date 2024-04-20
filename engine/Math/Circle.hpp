#pragma once

#include "Vec2.hpp"
#include "Aabb.hpp"

bool circleContains(Vec2 circleCenter, float circleRadius, Vec2 position);
bool circleAabbCollision(Vec2 circleCenter, float circleRadius, const Aabb& aabb);