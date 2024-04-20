#pragma once

#include <vector>
#include <platformer/Blocks.hpp>
#include <platformer/Player.hpp>
#include <framework/Renderer2d.hpp>

struct GameRenderer {
	GameRenderer();

	void update();

	void renderBlock(const Block& block);
	void renderBlocks(const std::vector<Block>& blocks);
	void renderSpike(const Spike& spike);
	void renderPlatform(const Platform& platform);
	void renderPlayer(const Player& player);
	void renderDoubleJumpOrb(const Vec2 position);
	void renderDoubleJumpOrb(const DoubleJumpOrb& doubleJumpOrb);

	void renderGrid(f32 smallCellSize);

	Renderer2d renderer;

	Vao gridVao;
	ShaderProgram& gridShader;
};