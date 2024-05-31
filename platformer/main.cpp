#include <engine/Engine.hpp>
#include <engine/EngineUpdateLoop.hpp>
#include <platformer/Audio/Audio.hpp>
#include <platformer/Assets.hpp>
#include <platformer/MainLoop.hpp>

#include <iostream>
int main() {
	/*Engine::initAll(Window::Settings{ .multisamplingSamplesPerPixel = 4 });*/
	Engine::initAll(Window::Settings{});
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

#ifdef FINAL_RELEASE

#ifdef WIN32
#include <Windows.h>
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	return main();
}
#endif

#endif