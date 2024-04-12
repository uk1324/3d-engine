#pragma once

#include <vector>
#include <platformer/Blocks.hpp>
#include <platformer/Player.hpp>
#include <framework/Renderer2d.hpp>

struct GameRenderer {
	GameRenderer();

	void update();

	void renderBlocks(const std::vector<Block>& blocks, f32 cellSize);
	void renderPlayer(const Player& player, const PlayerSettings& settings);

	void renderGrid(f32 smallCellSize);

	Renderer2d renderer;

	Vao gridVao;
	ShaderProgram& gridShader;
};