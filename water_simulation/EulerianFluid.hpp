#pragma once

#include <engine/Math/Vec2.hpp>
#include <Span2d.hpp>
#include <vector>

class EulerianFluid {
public:
	EulerianFluid(Vec2T<i64> gridSize, float cellSpacing, float overRelaxation = 1.9f, float density = 1000.0f);

	auto integrate(float dt, float gravity) -> void;
	//struct OpenSides {
	//	bool xMinus, xPlus, yMinus, yPlus;
	//	i64 count() const;
	//};
	//OpenSides getOpenSides(i64 x, i64 y) const;
	auto solveIncompressibility(i32 solverIterations, float dt) -> void;
	void computeDivergence();

	float sampleField(Span2d<const float> field, Vec2 pos, Vec2 cellOffset);
	float sampleFieldVelX(const std::vector<float>& field, Vec2 pos);
	float sampleFieldVelY(const std::vector<float>& field, Vec2 pos);
	Vec2 sampleVel(Vec2 pos);
	std::optional<float> sampleQuantity(Span2d<const float> field, Vec2 pos);

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
	float isWallFloat(i64 x, i64 y) const;
	// Doesn't work at grid boundaries.
	void removeVelocityAround(i64 x, i64 y);

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
	/*std::vector<bool> isWallValues;*/
	std::vector<float> isWallValues;
	std::vector<float> advectedQuantityOld;
	std::vector<float> divergence;
};
// @Performance:
// Changing the isWall to floats didn't change the performance. I though that maybe transfering the bools between ALUs or using std::vector<bool> might be slowing down the solver. After implementing the change the performance didn't change. One issue might be that using floats takes up more cache memory. It might also be the case that the compiler can optimize the code so it loads the bools directly into the FP ALU.

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
