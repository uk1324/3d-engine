#pragma once

#include <engine/Math/Vec2.hpp>
#include <vector>
#include "Array2d.hpp"
#include "PerlinNoise.hpp"

struct OrthogonalDemo {
	OrthogonalDemo();
	void update();
	void update2();

	Array2d<Vec2> grid = Array2d<Vec2>(400, 400);
	Vec2 gridPos(Vec2 vt);

	Array2d<f32> array = Array2d<f32>(200, 200);
	struct Point {
		Vec2 p;
		std::vector<Vec2> history;
		f32 direction;
	};

	bool paused = false;
	std::vector<Vec2> isolines;
	std::vector<Point> points;
	f32 sampleNoise(Vec2 p);
	Vec2 sampleVectorField(Vec2 p);
	PerlinNoise noise;
};