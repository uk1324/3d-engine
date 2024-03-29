#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <vector>
#include <water_simulation/FirstOrderSystemGraph.hpp>
#include <water_simulation/SecondOrderSystemGraph.hpp>
#include <imgui/imgui.h>
#include <water_simulation/SettingsManager.hpp>
#include <water_simulation/FunctionPlotter2d.hpp>
#include <water_simulation/RenderWindow.hpp>

struct GraphDemo {
	GraphDemo();
	void update();

	void openHelpWindow();
	void helpWindow();

	/*enum class State {
		FIRST_ORDER_SYSTEM,
		SECOND_ORDER_SYSTEM,
	};*/
	//State state = State::FIRST_ORDER_SYSTEM;
	Texture texture;
	FirstOrderSystemGraph firstOrderSystem;
	SecondOrderSystemGraph secondOrderSystem;
	FunctionPlotter2d plotter;

	RenderWindow window;
};