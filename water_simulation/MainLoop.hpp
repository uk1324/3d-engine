#pragma once

#include <water_simulation/EulerianFluidDemo.hpp>
#include <water_simulation/FlipPicFluidDemo.hpp>

struct MainLoop {
	MainLoop();

	void update();

	EulerianFluidDemo eulerianFluidDemo;
	FlipPicFluidDemo flipPicFluidDemo;
};