#pragma once

#include <platformer/GameRenderer.hpp>

struct Menu {
	Menu(GameRenderer& renderer);

	void update(GameRenderer& renderer);
	void drawTextCentered(GameRenderer& renderer, std::string_view text, Vec2 position, f32 height);
	Aabb getTextAabb(std::string_view text, Vec2 position, f32 height);

	Camera camera;
	GameRenderer& renderer;
};