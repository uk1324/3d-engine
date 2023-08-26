#include <Put.hpp>
#include <FixedUpdateLoop.hpp>
#include <engine/Engine.hpp>
#include <game/Game.hpp>

int main() {
	Engine::initAll(Window::Settings{
		.multisamplingSamplesPerPixel = 16
	});

	Game game;

	FixedUpdateLoop fixedUpdateLoop(60.0);
	while (!Window::shouldClose() && fixedUpdateLoop.isRunning()) {
		Engine::updateFrameStart();
		game.update();
		Engine::updateFrameEnd();
	}

	Engine::terminateAll();
}