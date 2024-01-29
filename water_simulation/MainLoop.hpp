#pragma once

#include <water_simulation/EulerianFluidDemo.hpp>
#include <water_simulation/FlipPicFluidDemo.hpp>
#include <water_simulation/PolynomialInterpolationDemo.hpp>

struct MainLoop {
	MainLoop();

	void update();

	EulerianFluidDemo eulerianFluidDemo;
	FlipPicFluidDemo flipPicFluidDemo;
	PolynomialInterpolationDemo polynomialInterpolationDemo;
};