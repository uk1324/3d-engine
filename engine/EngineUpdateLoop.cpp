#include <engine/EngineUpdateLoop.hpp>
#include <engine/Engine.hpp>

EngineUpdateLoop::EngineUpdateLoop(double fps) 
	: fixedUpdateLoop(fps) {}

bool EngineUpdateLoop::isRunning() {
	if (!firstFrame) {
		Engine::updateFrameEnd();
	} else {
		firstFrame = false;
	}

	// Update frame end first because isRunning blocks.
	const auto running = fixedUpdateLoop.isRunning() && !Window::shouldClose();
	Engine::updateFrameStart();
	return running;
}
