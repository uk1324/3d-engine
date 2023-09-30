#include <engine/EngineUpdateLoop.hpp>
#include <engine/Engine.hpp>
#include <water_rendering/MainLoop.hpp>

#include <engine/Input/Input.hpp>
#include <engine/Utils/Dbg.hpp>

int main() {
	Engine::initAll(Window::Settings{ .multisamplingSamplesPerPixel = 4 });

	EngineUpdateLoop loop(60.0);
	auto mainLoop = MainLoop::make();

	while (loop.isRunning()) {
		mainLoop.update();
	}

	Engine::terminateAll();
}