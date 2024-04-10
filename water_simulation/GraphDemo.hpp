#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <vector>
#include <water_simulation/FirstOrderSystemGraph.hpp>
#include <water_simulation/Continous2dSystemVisualization.hpp>
#include <imgui/imgui.h>
#include <water_simulation/SettingsManager.hpp>
#include <water_simulation/FunctionPlotter2d.hpp>
#include <water_simulation/RenderWindow.hpp>
#include <framework/Renderer2d.hpp>

struct GraphDemo {
	GraphDemo();
	void update();

	void openHelpWindow();
	void helpWindow();

	Texture texture;
	FirstOrderSystemGraph firstOrderSystem;
	Continous2dSystemVisualization secondOrderSystem;
	FunctionPlotter2d plotter;

	RenderWindow window;

	Renderer2d renderer2d;
};