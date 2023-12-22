#pragma once

#include <Array2d.hpp>
#include <vector>

struct FlipPicFluid {
	FlipPicFluid(Vec2T<i64> gridSize);

	void update(float dt);
	void integrate(float dt);
	void resolveCollisions();
	void transferVelocitiesToVelocityField();
	void makeIncompressible();

	Vec2T<i64> gridSize;
	float gridCellSize = 0.1f;
	float particleRadius;

	std::vector<Vec2> particlePositions;
	std::vector<Vec2> particleVelocities;

	i64 particleCount();
	Array2d<float> velXValues;
	Array2d<float> velXWeights;
	Array2d<float> velYValues;
	Array2d<float> velYWeights;
	Array2d<float> velX;
	Array2d<float> velY;
	std::vector<bool> isWallValues;

	void setIsWall(i64 x, i64 y, bool value);
	bool isWall(i64 x, i64 y);
};