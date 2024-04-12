#include <engine/Engine.hpp>
#include <engine/EngineUpdateLoop.hpp>
#include <platformer/Game.hpp>

int main() {
	Engine::initAll(Window::Settings{ .multisamplingSamplesPerPixel = 4 });

	EngineUpdateLoop loop(60.0);
	auto game = Game();

	while (loop.isRunning()) {
		game.update();
	}

	Engine::terminateAll();
}