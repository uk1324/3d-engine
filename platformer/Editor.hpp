#pragma once

#include <platformer/GameRenderer.hpp>

struct Editor {
	Editor();

	void update(f32 dt);
	void render(GameRenderer& renderer, f32 cellSize);

	std::optional<Vec2> moveGrabStartWorldPos;

	Camera camera;
};