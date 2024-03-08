#include <water_simulation/MainLoop.hpp>

#include <Put.hpp>
#include <engine/Math/RootFinding/bisection.hpp>
#include <engine/Math/RootFinding/newton.hpp>
#include <engine/Math/Angles.hpp>
using namespace RootFinding;

#include <imgui/implot.h>

MainLoop::MainLoop() 
	: eulerianFluidDemo(EulerianFluidDemo::make()) {
}


void MainLoop::update() {
	graphDemo.update();
}