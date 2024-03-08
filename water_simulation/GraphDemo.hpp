#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <vector>
#include <water_simulation/FirstOrderSystemGraph.hpp>
#include <imgui/imgui.h>

struct GraphDemo {
	GraphDemo();
	void update();

	float spacing = 0.1f;
	float damping = 1.0f;
	std::vector<Vec2> points;
	struct TestPoint {
		Vec2 pos;
		std::vector<Vec2> history;
	};
	std::vector<TestPoint> midpointPoints;
	std::vector<TestPoint> eulerPoints;
	std::vector<TestPoint> rungeKutta4Points;
	bool paused = false;

	FirstOrderSystemGraph firstOrderSystem;
};