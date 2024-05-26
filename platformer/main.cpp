#include <engine/Engine.hpp>
#include <engine/EngineUpdateLoop.hpp>
#include <platformer/Game.hpp>
#include <platformer/Audio/Audio.hpp>
#include <platformer/Assets.hpp>

int main() {
	Engine::initAll(Window::Settings{ .multisamplingSamplesPerPixel = 4 });
	Audio::init();
	{
		Assets assets = loadAssets();
		::assets = &assets;
		// Loading it here so they are destroyed in the correct order.

		EngineUpdateLoop loop(60.0);
		auto game = Game();

		while (loop.isRunning()) {
			game.update();
		}
	}
	Audio::deinit();
	Engine::terminateAll();
}