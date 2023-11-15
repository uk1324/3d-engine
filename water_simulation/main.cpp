#include <engine/EngineUpdateLoop.hpp>
#include <engine/Engine.hpp>
#include <water_simulation/MainLoop.hpp>

int main() {
	Engine::initAll(Window::Settings{.multisamplingSamplesPerPixel = 4 });

	EngineUpdateLoop loop(60.0);
	auto mainLoop = MainLoop::make();

	while (loop.isRunning()) {
		mainLoop.update();
	}

	Engine::terminateAll();
}