#include <platformer/GameRenderer.hpp>
#include <framework/Dbg.hpp>
#include <framework/ShaderManager.hpp>
#include <framework/Instancing.hpp>
#include <platformer/Constants.hpp>
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

void GameRenderer::renderBlock(const Block& block) {
	using namespace BlockCollisionDirections;

	const auto color = Color3::GREEN;

	f32 width = 2.0f;

	Dbg::drawFilledAabb(block.position, block.position + Vec2(constants().cellSize), Color3::WHITE / 20.0f);

	if (block.collisionDirections & D) {
		Dbg::drawLine(
			Vec2(block.position),
			Vec2(block.position) + Vec2(constants().cellSize, 0.0f),
			color,
			width);
	}

	if (block.collisionDirections & U) {
		Dbg::drawLine(
			Vec2(block.position) + Vec2(0.0f, constants().cellSize),
			Vec2(block.position) + Vec2(constants().cellSize, constants().cellSize),
			color,
			width);
	}

	if (block.collisionDirections & L) {
		Dbg::drawLine(
			Vec2(block.position),
			Vec2(block.position) + Vec2(0.0f, constants().cellSize),
			color,
			width);
	}

	if (block.collisionDirections & R) {
		Dbg::drawLine(
			Vec2(block.position) + Vec2(constants().cellSize, 0.0f),
			Vec2(block.position) + Vec2(constants().cellSize, constants().cellSize),
			color,
			width);
	}
}

void GameRenderer::renderBlocks(const std::vector<Block>& blocks) {

	for (const auto& block : blocks) {
		renderBlock(block);
	}
}

void GameRenderer::renderSpike(const Spike& spike) {
	Dbg::drawFilledAabb(
		spike.hitbox.min,
		spike.hitbox.max,
		Color3::RED);
}

void GameRenderer::renderPlatform(const Platform& platform) {
	Dbg::drawLine(platform.position, platform.position + Vec2(constants().cellSize, 0.0f), Color3::WHITE / 2.0f, 2.0f);
}

void GameRenderer::renderPlayer(const Player& player) {
	Dbg::drawAabb(
		player.position - constants().playerSize / 2.0f,
		player.position + constants().playerSize / 2.0f,
		Color3::WHITE,
		2.0f);
}

void GameRenderer::renderDoubleJumpOrb(const Vec2 position) {
	Dbg::drawCircle(position, constants().doubleJumpOrbRadius, Color3::GREEN, 0.1f);
}

void GameRenderer::renderAttractingOrb(const Vec2 position) {
	Dbg::drawCircle(position, constants().attractingOrbRadius, Color3::CYAN, 0.1f);
}

void GameRenderer::renderDoubleJumpOrb(const DoubleJumpOrb& doubleJumpOrb) {
	const auto color = doubleJumpOrb.isActive() ? Color3::GREEN : Color3::WHITE / 2.0f;
	Dbg::drawCircle(doubleJumpOrb.position, constants().doubleJumpOrbRadius, color, 0.1f);
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
