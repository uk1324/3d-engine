#pragma once

#include <engine/Math/Vec3.hpp>
#include <engine/Math/Vec2.hpp>

#include <random>

// Cool library - https://github.com/Auburn/FastNoiseSIMD

// Value noise interpolates between random values on a grid which produces not very good results.
// Perlin noise interpolates between the dot products of gradient vectors and the point.
// This produces a smoother result.

// This could be optimized by making a function for batch generation of noise.
// Also could make specialized functions for lower dimensional noise.
class PerlinNoise
{
public:
	PerlinNoise(uint64_t seed);

	float value3d(const Vec3& p) const;
	float value2d(const Vec2& p) const;

	// lacunarity - how much is the input scaled on each octave
	// persistence - how much is the output scaled on each octave 
	float accumulatedValue3d(const Vec3& p, int octaves, float lacunarity, float persistence) const;
	float accumulatedValue2d(const Vec2& p, int octaves, float lacunarity, float persistence) const;

	float value3d01(const Vec3& p) const;
	float value2d01(const Vec2& p) const;

private:
	// Returns values in range -1 to 1.
	float at(const Vec3& p) const;

	// Hash function so there doesn't need to be a large permutation table to get many random points.
	int hash(int x, int y, int z) const;

public:
	static float accumulatedValueMax(int octaves, float persistence);

private:
	static float smoothstep(float t);

private:
	static constexpr size_t GRADIENT_TABLE_SIZE = 256;

private:
	// Not using std::array because the bounds checks make it way too slow in debug mode.
	Vec3 m_gradients[GRADIENT_TABLE_SIZE];
	// Permutation table size is twice as big the gradient table because the permutation hash function returns values in range 0 to 511.
	uint8_t m_permutations[GRADIENT_TABLE_SIZE * 2];
};