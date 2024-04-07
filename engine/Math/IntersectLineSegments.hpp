#pragma once

#include "Vec2.hpp"
#include <vector>
#include <span>

/*
The arrays should contain the endpoints of lines like this [s0, e0, s1, e1, ...] where s is the start and e is the end.
Segments from the same array are not intersected.
*/
void intersectLineSegments(std::span<const Vec2> segmentsA, std::span<const Vec2> segmentsB, std::vector<Vec2>& intersectionsOut);