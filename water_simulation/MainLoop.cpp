#include <water_simulation/MainLoop.hpp>

MainLoop::MainLoop() 
	: eulerianFluidDemo(EulerianFluidDemo::make()) {}

void MainLoop::update() {
	//eulerianFluidDemo.update();
	flipPicFluidDemo.update();
}