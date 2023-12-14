#pragma once

#include <Span2d.hpp>
#include <vector>

// Calculates isoline polygons.
std::vector<std::vector<Vec2>> marchingSquares(Span2d<const float> grid, bool pixelPerfect, bool conntectDiagonals, float boundaryValue);

struct MarchingSquaresLine {
	Vec2 a, b;
};
// Calculates the lines which make the isolines.
/*
Handling points in the grid where the values is undefined.
If the value is set to std::numericlimits<float>::quiet_NaN() or any other nan then the algorithm disables linear interpolation around that point.

Another way to handle them is to set the value to a really big or really small value to bring it closer or further from the center of the wall cell. Setting to infinity currently doesn't work. It causes isolines only to appear on one side of the wall top or bottom, left or right.
*/
void marchingSquares2(std::vector<MarchingSquaresLine>& output, Span2d<const float> grid, float boundaryValue, bool lerp);