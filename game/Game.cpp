#include <game/Game.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Window.hpp>

// TODO: In perspective projection make the w of the output vector smaller than 1. Instead of copying z to w copy 1 / z to w
// TODO: Lenses and reflectors with rays and maybe postprocessing.
Game::Game() 
	: renderer(Renderer::make()) {
	Window::disableCursor();
}

// [0, 0], [0, 1], [1, 0] to any triangle. Translate one vertex to 0. Transform the others using a matrix.
void Game::update() {
	renderer.update();

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	if (Input::isKeyDown(KeyCode::P)) {
		Window::toggleCursor();
	}
}