#include <engine/Engine.hpp>
#include <engine/EngineUpdateLoop.hpp>
#include <platformer/Audio/Audio.hpp>
#include <platformer/Assets.hpp>
#include <platformer/MainLoop.hpp>

int main() {
	Engine::initAll(Window::Settings{ .multisamplingSamplesPerPixel = 4 });
	Audio::init();
	{
		Assets assets = loadAssets();
		::assets = &assets;
		// Loading it here so they are destroyed in the correct order.

		EngineUpdateLoop loop(60.0);
		MainLoop mainLoop;

		while (loop.isRunning()) {
			mainLoop.update();
		}
	}
	Audio::deinit();
	Engine::terminateAll();
}