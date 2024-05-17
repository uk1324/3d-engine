#pragma once

#include <water_simulation/EulerianFluidDemo.hpp>
#include <water_simulation/FlipPicFluidDemo.hpp>
#include <water_simulation/PolynomialInterpolationDemo.hpp>
#include <water_simulation/GraphDemo.hpp>
#include <water_simulation/SpecialFunctionsDemo.hpp>
#include <water_simulation/IntegrationDemo.hpp>
#include <water_simulation/MatrixDemo.hpp>
#include <water_simulation/OrthogonalDemo.hpp>

struct MainLoop {
	MainLoop();

	void update();

	MatrixDemo matrixDemo;
	OrthogonalDemo orthogonalDemo;
	/*EulerianFluidDemo eulerianFluidDemo;
	FlipPicFluidDemo flipPicFluidDemo;
	PolynomialInterpolationDemo polynomialInterpolationDemo;*/
	GraphDemo graphDemo;
	/*SpecialFunctionsDemo specialFunctionsDemo;
	IntegrationDemo integrationDemo;*/
};