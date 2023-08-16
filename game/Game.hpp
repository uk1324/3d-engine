#pragma once

#include <game/Renderer.hpp>

struct Game {
	Game();

	void update();

	Renderer renderer;
};