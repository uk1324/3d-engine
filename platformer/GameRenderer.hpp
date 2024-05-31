#pragma once

#include <vector>
#include <platformer/Blocks.hpp>
#include <platformer/Player.hpp>
#include <framework/Renderer2d.hpp>
#include <framework/FontRendering/FontRenderer.hpp>
#include <platformer/Shaders/attractingOrbData.hpp>
#include <platformer/Shaders/doubleJumpOrbData.hpp>

struct GameRenderer {
	GameRenderer();

	void update();
	void updateAnimatitons();

	void renderBlock(const Block& block);
	void renderBlocks(const std::vector<Block>& blocks);
	void renderBlockOutline(Vec2 min, Vec2 max);
	void renderBlockOutlines(const Array2d<BlockType>& roomBlockGrid, Vec2T<i32> roomPosition);
	void renderSpikes(const Array2d<BlockType>& roomBlockGrid, Vec2T<i32> roomPosition);
	void renderSpike(const Spike& spike);
	void renderPlatform(const Platform& platform);
	void renderPlayer(const Player& player);
	void renderPlayerFull(const Player& player);
	void renderDoubleJumpOrb(const Vec2 position);
	void renderAttractingOrb(const Vec2 position);

	std::vector<AttractingOrbInstance> attractingOrbInstances;
	void addAttractingOrbs(const std::vector<AttractingOrb>& attractingOrbs, Vec2 playerPos);
	void renderAttractingOrbs();

	std::vector<DoubleJumpOrbInstance> doubleJumpOrbInstances;
	void addDoubleJumpOrbs(const std::vector<DoubleJumpOrb>& doubleJumpOrbs);
	void renderDoubleJumpOrbs();

	void renderDoubleJumpOrb(const DoubleJumpOrb& doubleJumpOrb);

	void renderGrid(f32 smallCellSize);
	void renderBackground();

	Renderer2d renderer;
	FontRenderer fontRenderer;

	Vao gridVao;
	ShaderProgram& gridShader;

	Vao backgroundVao;
	ShaderProgram& backgroundShader;

	ShaderProgram& blocksShader;

	Vao playerVao;
	ShaderProgram& playerShader;

	Vao spikeCenterVao;
	ShaderProgram& spikeCenterShader;

	Vao spikeOpenCornerVao;
	ShaderProgram& spikeOpenCornerShader;

	Vao spikeClosedCornerVao;
	ShaderProgram& spikeClosedCornerShader;

	Vao attractingOrbVao;
	ShaderProgram& attractingOrbShader;

	Vao doubleJumpOrbVao;
	ShaderProgram& doubleJumpOrbShader;

	Font font;

	f32 backgroundElapsed = 0.0f;
};