#pragma once

#include <engine/Math/Vec2.hpp>
#include <Span2d.hpp>
#include <vector>

class EulerianFluid {
public:
	EulerianFluid(Vec2T<i64> gridSize, float cellSpacing, float overRelaxation = 1.9f, float density = 1000.0f);

	auto integrate(float dt, float gravity) -> void;
	auto solveIncompressibility(i32 solverIterations, float dt) -> void;

	float sampleField(Span2d<const float> field, Vec2 pos, Vec2 cellOffset);
	float sampleFieldVelX(const std::vector<float>& field, Vec2 pos);
	float sampleFieldVelY(const std::vector<float>& field, Vec2 pos);
	Vec2 sampleVel(Vec2 pos);
	float sampleQuantity(Span2d<const float> field, Vec2 pos);

	void advectVelocity(float dt);
	// @Performance: Would it be faster in some cases to have a single "vector" of quantites that get advected all at once. It would need less computation, but there would probably be issues with caching.
	void advectQuantity(Span2d<float> quantity, float dt);
	auto update(float dt, float gravity, i32 solverIterations) -> void;
	template<typename T>
	auto at(std::vector<T>& vec, i64 x, i64 y) -> T&;
	template<typename T>
	Span2d<T> spanFrom(std::vector<T>& vec);
	template<typename T>
	Span2d<const T> spanFrom(const std::vector<T>& vec) const;
	auto setIsWall(i64 x, i64 y, bool value) -> void;
	auto isWall(i64 x, i64 y) const -> bool;

	float density;
	Vec2T<i64> gridSize;
	float cellSpacing;
	// Speeds up convergence https://en.wikipedia.org/wiki/Successive_over-relaxation.
	float overRelaxation;
	// @Performance: Could double buffer the velocities instead of copying them. Would need to profile to see if the extra level of indirection has any impact on performance.
	std::vector<float> velX;
	std::vector<float> velY;
	std::vector<float> oldVelX;
	std::vector<float> oldVelY;
	std::vector<float> pressure;
	std::vector<bool> isWallValues;
	std::vector<float> divergence;
	std::vector<float> advectedQuantityOld;
	/*
	Velocities v around a point [x, y] lie on a staggered grid.
			v0[x, y + 1]
	v0[x, y]            v0[x + 1, y]
			  v0[x, y]
	The cells are cellSpacing apart from eachother.
	To get the velocity at the point [x, y] these values have to be interpolated.
	Other values don't need to be interpolated.
	*/
};

template<typename T>
auto EulerianFluid::at(std::vector<T>& vec, i64 x, i64 y) -> T& {
	return vec[y * gridSize.x + x];
}

template<typename T>
Span2d<T> EulerianFluid::spanFrom(std::vector<T>& vec) {
	return Span2d<T>(vec.data(), gridSize.x, gridSize.y);
}

template<typename T>
Span2d<const T> EulerianFluid::spanFrom(const std::vector<T>& vec) const {
	return Span2d<const T>(vec.data(), gridSize.x, gridSize.y);
}
