#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <vector>
#include <water_simulation/FirstOrderSystemGraph.hpp>
#include <water_simulation/SecondOrderSystemGraph.hpp>
#include <imgui/imgui.h>

struct GraphDemo {
	GraphDemo();
	void update();

	void openHelpWindow();
	void helpWindow();

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

	enum class State {
		FIRST_ORDER_SYSTEM,
		SECOND_ORDER_SYSTEM,
	};
	State state = State::FIRST_ORDER_SYSTEM;

	FirstOrderSystemGraph firstOrderSystem;
	SecondOrderSystemGraph secondOrderSystem;
};