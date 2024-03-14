#include <water_simulation/MainLoop.hpp>

#include <Put.hpp>
#include <engine/Math/RootFinding/bisection.hpp>
#include <engine/Math/RootFinding/newton.hpp>
#include <engine/Math/Angles.hpp>
using namespace RootFinding;

#include <imgui/implot.h>
//#include <Unwrap.hpp>

MainLoop::MainLoop() 
	: eulerianFluidDemo(EulerianFluidDemo::make()) {

	//auto f = [](float x) {
	//	//return pow(x, 2.0f) - 1.0f;
	//	return sin(x);
	//};
	////unwrap()
	//const auto out = bisect<double>(2.5, 4.0f, +f, 10000, 0.00001f);

	//put("%", out->input);

	//auto f = [](double x) {
	//	//return pow(x, 2.0f) - 1.0f;
	//	return cos(x) - x;
	//};

	//auto fPrim = [](double x) {
	//	return -sin(x) - 1.0;
	//};
	////unwrap()
	//const auto out = newton<double>(PI<double> / 4.0, +f, +fPrim, i64(1000), 0.00000001);

	//put("%", out->approximateRoot);
}


void MainLoop::update() {
	//polynomialInterpolationDemo.update();
	//eulerianFluidDemo.update();
	graphDemo.update();
	//specialFunctionsDemo.update();
	//eulerianFluidDemo.update();
	//integrationDemo.update();
}