#pragma once

#include <platformer/GameRenderer.hpp>

struct UiLayout {
	// size goes from 0 to 1 where 1 is the full screen.
	f32 totalSizeY = 0.0f;
	struct Block {
		f32 yPosition;
		f32 sizeY;
		Aabb aabb;
	};
	std::vector<Block> blocks;

	void addPadding(f32 sizeY);
	i32 addBlock(f32 sizeY);
};

struct Menu {
	Menu(GameRenderer& renderer);

	struct Text {
		std::string_view text;
		Vec2 position = Vec2(0.0f);
		f32 sizeY;
		Aabb aabb = Aabb(Vec2(0.0f), Vec2(0.0f));
		f32 hoverAnimationT = 0.0f;
	};
	f32 totalHeight = 0.0f;
	std::vector<Text> texts;

	void update(GameRenderer& renderer);
	void drawTextCentered(GameRenderer& renderer, std::string_view text, Vec2 position, f32 height);
	Aabb getTextAabb(std::string_view text, Vec2 position, f32 height);

	Camera camera;
	GameRenderer& renderer;
};