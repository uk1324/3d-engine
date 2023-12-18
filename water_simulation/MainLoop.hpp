#pragma once

#include <water_simulation/EulerianFluidDemo.hpp>

struct MainLoop {
	MainLoop();

	void update();

	EulerianFluidDemo eulerianFluidDemo;
};