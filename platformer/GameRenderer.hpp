#pragma once

#include <vector>
#include <platformer/Blocks.hpp>
#include <platformer/Player.hpp>
#include <framework/Renderer2d.hpp>
#include <platformer/Shaders/attractingOrbData.hpp>

struct GameRenderer {
	GameRenderer();

	void update();

	void renderBlock(const Block& block);
	void renderBlocks(const std::vector<Block>& blocks);
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
	void renderDoubleJumpOrb(const DoubleJumpOrb& doubleJumpOrb);

	void renderGrid(f32 smallCellSize);
	void renderBackground();

	Renderer2d renderer;

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

	f32 backgroundElapsed = 0.0f;
};