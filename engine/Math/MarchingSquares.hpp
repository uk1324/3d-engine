#pragma once

#include <Span2d.hpp>
#include <vector>

// Calculates isoline polygons.
std::vector<std::vector<Vec2>> marchingSquares(Span2d<const float> grid, bool pixelPerfect, bool conntectDiagonals, float boundaryValue);

struct MarchingSquaresLine {
	Vec2 a, b;
};
// Calculates the lines which make the isolines.
std::vector<MarchingSquaresLine> marchingSquares2(Span2d<const float> grid, float boundaryValue, bool lerp);