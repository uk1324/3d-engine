#include <platformer/GameRenderer.hpp>
#include <framework/Dbg.hpp>
#include <framework/ShaderManager.hpp>
#include <framework/Instancing.hpp>
#include <platformer/Shaders/gridData.hpp>
#include <engine/Math/Color.hpp>
#include <FileIo.hpp>

GameRenderer::GameRenderer()
	: renderer(Renderer2d::make())
	// TODO: maybe allow the manager to take one source one path to source.
	, gridShader(MAKE_GENERATED_SHADER(GRID))
	, gridVao(createInstancingVao<GridShader>(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo)) {
}

void GameRenderer::update() {
	renderer.update();
}

void GameRenderer::renderBlock(const Block& block, f32 cellSize) {
	using namespace BlockCollisionDirections;

	const auto color = Color3::GREEN;

	f32 width = 2.0f;

	Dbg::drawFilledAabb(block.position, block.position + Vec2(cellSize), Color3::WHITE / 20.0f);

	if (block.collisionDirections & D) {
		Dbg::drawLine(
			Vec2(block.position),
			Vec2(block.position) + Vec2(cellSize, 0.0f),
			color,
			width);
	}

	if (block.collisionDirections & U) {
		Dbg::drawLine(
			Vec2(block.position) + Vec2(0.0f, cellSize),
			Vec2(block.position) + Vec2(cellSize, cellSize),
			color,
			width);
	}

	if (block.collisionDirections & L) {
		Dbg::drawLine(
			Vec2(block.position),
			Vec2(block.position) + Vec2(0.0f, cellSize),
			color,
			width);
	}

	if (block.collisionDirections & R) {
		Dbg::drawLine(
			Vec2(block.position) + Vec2(cellSize, 0.0f),
			Vec2(block.position) + Vec2(cellSize, cellSize),
			color,
			width);
	}
}

void GameRenderer::renderBlocks(const std::vector<Block>& blocks, f32 cellSize) {

	for (const auto& block : blocks) {
		renderBlock(block, cellSize);
	}
}

void GameRenderer::renderSpike(const Spike& spike) {
	Dbg::drawAabb(
		spike.hitbox.min,
		spike.hitbox.max,
		Color3::RED,
		1.0f);
}

void GameRenderer::renderPlayer(const Player& player, const PlayerSettings& settings) {
	Dbg::drawAabb(
		player.position - settings.size / 2.0f,
		player.position + settings.size / 2.0f,
		Color3::WHITE,
		2.0f);
}

void GameRenderer::renderGrid(f32 smallCellSize) {
	gridShader.use();
	std::vector<GridInstance> instances;
	instances.push_back(GridInstance{
		.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
		.cameraZoom = renderer.camera.zoom,
		.smallCellSize = smallCellSize
	});
	drawInstances(gridVao, renderer.instancesVbo, instances, [](usize count) {
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count);
	});
}
