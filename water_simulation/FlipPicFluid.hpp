#pragma once

#include <Array2d.hpp>
#include <vector>

struct FlipPicFluid {
	FlipPicFluid(Vec2T<i64> gridSize);

	void update();

	std::vector<Vec2> particlePositions;
	std::vector<Vec2> particleVelocities;
	Vec2T<i64> gridSize;
	Array2d<Vec2> velocityField;
	std::vector<bool> isNotWall;
};