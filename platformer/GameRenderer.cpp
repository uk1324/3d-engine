#include <platformer/GameRenderer.hpp>
#include <framework/Dbg.hpp>
#include <framework/ShaderManager.hpp>
#include <engine/Math/Angles.hpp>
#include <framework/Instancing.hpp>
#include <platformer/Constants.hpp>
#include <platformer/Shaders/gridData.hpp>
#include <platformer/Shaders/backgroundData.hpp>
#include <platformer/Shaders/blocksData.hpp>
#include <platformer/Shaders/playerData.hpp>
#include <platformer/Shaders/spikeCenterData.hpp>
#include <platformer/Shaders/spikeOpenCornerData.hpp>
#include <platformer/Shaders/spikeClosedCornerData.hpp>
#include <engine/Math/Color.hpp>
#include <FileIo.hpp>

GameRenderer::GameRenderer()
	: renderer(Renderer2d::make())
	// TODO: maybe allow the manager to take one source one path to source.
	, gridShader(MAKE_GENERATED_SHADER(GRID))
	, gridVao(createInstancingVao<GridShader>(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo))
	, backgroundShader(MAKE_GENERATED_SHADER(BACKGROUND))
	, backgroundVao(createInstancingVao<BackgroundShader>(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo))
	, blocksShader(MAKE_GENERATED_SHADER(BLOCKS))
	, playerVao(createInstancingVao<PlayerShader>(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo))
	, playerShader(MAKE_GENERATED_SHADER(PLAYER))
	, spikeCenterVao(createInstancingVao<SpikeCenterShader>(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo))
	, spikeCenterShader(MAKE_GENERATED_SHADER(SPIKE_CENTER))
	, spikeOpenCornerVao(createInstancingVao<SpikeOpenCornerShader>(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo))
	, spikeOpenCornerShader(MAKE_GENERATED_SHADER(SPIKE_OPEN_CORNER))
	, spikeClosedCornerVao(createInstancingVao<SpikeClosedCornerShader>(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo))
	, spikeClosedCornerShader(MAKE_GENERATED_SHADER(SPIKE_CLOSED_CORNER)) {
}

void GameRenderer::update() {
	
	renderer.update();
	backgroundElapsed += 1.0f / 60.0f;
}

const auto BLOCK_COLOR = Color3::WHITE / 10.0f;

void GameRenderer::renderBlock(const Block& block) {
	using namespace BlockCollisionDirections;

	/*const auto color = Color3::GREEN;*/
	const auto color = BLOCK_COLOR;


	f32 width = 2.0f;

	//Dbg::drawFilledAabb(block.position, block.position + Vec2(constants().cellSize), Color3::WHITE / 20.0f);

	/*if (block.collisionDirections & D) {
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
	}*/

	float b = 2.0f;
	if (block.collisionDirections & D) {
		Dbg::drawFilledAabb(block.position, block.position + Vec2(constants().cellSize, b), color);
		/*Dbg::drawLine(
			Vec2(block.position),
			Vec2(block.position) + Vec2(constants().cellSize, 0.0f),
			color,
			width);*/
	}

	const auto p = block.position + Vec2(constants().cellSize);
	if (block.collisionDirections & U) {
		Dbg::drawFilledAabb(p - Vec2(constants().cellSize, b), p, color);
		/*Dbg::drawLine(
			Vec2(block.position) + Vec2(0.0f, constants().cellSize),
			Vec2(block.position) + Vec2(constants().cellSize, constants().cellSize),
			color,
			width);*/
	}

	if (block.collisionDirections & L) {
		Dbg::drawFilledAabb(block.position, block.position + Vec2(b, constants().cellSize), color);
		/*Dbg::drawLine(
			Vec2(block.position),
			Vec2(block.position) + Vec2(0.0f, constants().cellSize),
			color,
			width);*/
	}

	if (block.collisionDirections & R) {
		Dbg::drawFilledAabb(p - Vec2(b, constants().cellSize), p, color);
		/*Dbg::drawLine(
			Vec2(block.position) + Vec2(constants().cellSize, 0.0f),
			Vec2(block.position) + Vec2(constants().cellSize, constants().cellSize),
			color,
			width);*/
	}
}

void GameRenderer::renderBlocks(const std::vector<Block>& blocks) {

	for (const auto& block : blocks) {
		renderBlock(block);
	}
}

void GameRenderer::renderBlockOutlines(const Array2d<BlockType>& roomBlockGrid, Vec2T<i32> roomPosition) {
	for (i32 yi = 0; yi < roomBlockGrid.size().y; yi++) {
		for (i32 xi = 0; xi < roomBlockGrid.size().x; xi++) {
			if (roomBlockGrid(xi, yi) != BlockType::NORMAL) {
				continue;
			}
			auto get = [&roomBlockGrid](i32 x, i32 y) -> std::optional<u8> {
				if (x < 0 || y < 0 || x >= roomBlockGrid.size().x || y >= roomBlockGrid.size().y) {
					return std::nullopt;
				}
				if (roomBlockGrid(x, y) != BlockType::NORMAL) {
					return std::nullopt;
				}
				return getBlockCollisionDirections(roomBlockGrid, x, y);
			};

			const auto aboveType = get(xi, yi + 1);
			const auto belowType = get(xi, yi - 1);
			const auto rightType = get(xi + 1, yi);
			const auto leftType = get(xi - 1, yi);

			const auto type = getBlockCollisionDirections(roomBlockGrid, xi, yi);
			using namespace BlockCollisionDirections;
			const auto cellBottomLeft = Vec2(xi + roomPosition.x, yi + roomPosition.y) * constants().cellSize;
			const auto cellTopRight = cellBottomLeft + Vec2(constants().cellSize);
			//Dbg::drawAabb(cellBottomLeft, cellTopRight);
			float b = 2.0f;
			const auto color = Color3::WHITE / 10.0f;
			if (type & D) {
				Dbg::drawFilledAabb(cellBottomLeft, cellBottomLeft + Vec2(constants().cellSize, b), color);
			}
			if (type & U) {
				Dbg::drawFilledAabb(cellTopRight - Vec2(constants().cellSize, b), cellTopRight, color);
			}
			if (type & L) {
				Dbg::drawFilledAabb(cellBottomLeft, cellBottomLeft + Vec2(b, constants().cellSize), color);
			}
			if (type & R) {
				Dbg::drawFilledAabb(cellTopRight - Vec2(b, constants().cellSize), cellTopRight, color);
			}
			if (aboveType.has_value() && rightType.has_value() && *aboveType & R && *rightType & U) {
				Dbg::drawFilledAabb(cellTopRight - Vec2(b), cellTopRight, color);
			}
			if (aboveType.has_value() && leftType.has_value() && *aboveType & L && *leftType & U) {
				Dbg::drawFilledAabb(cellBottomLeft + Vec2(0.0f, constants().cellSize - b), cellBottomLeft + Vec2(b, constants().cellSize), color);
			}
			if (belowType.has_value() && leftType.has_value() && *belowType & L && *leftType & D) {
				Dbg::drawFilledAabb(cellBottomLeft, cellBottomLeft + Vec2(b), color);
			}
			if (belowType.has_value() && rightType.has_value() && *belowType & R && *rightType & D) {
				Dbg::drawFilledAabb(cellBottomLeft + Vec2(constants().cellSize - b, 0.0f), cellBottomLeft + Vec2(constants().cellSize, b), color);
			}

		}
	}
}

#include <imgui/imgui.h>

void GameRenderer::renderSpikes(const Array2d<BlockType>& roomBlockGrid, Vec2T<i32> roomPosition) {
	std::vector<SpikeCenterInstance> spikeCenters;
	std::vector<SpikeOpenCornerInstance> spikeOpenCorner;
	std::vector<SpikeClosedCornerInstance> spikeClosedCorner;

	for (i32 yi = 0; yi < roomBlockGrid.size().y; yi++) {
		for (i32 xi = 0; xi < roomBlockGrid.size().x; xi++) {
			const auto center = (Vec2(xi + roomPosition.x, yi + roomPosition.y) + Vec2(0.5f)) * constants().cellSize;

			auto addSpikeCenter = [&](f32 rotation, Vec2 normal) {
				const auto rot =
					Mat3x2::translate(-center) *
					Mat3x2::rotate(rotation) *
					Mat3x2::translate(center);
					const auto spike = makeSpikeLeft(xi, yi, roomPosition);
					auto aabb = spike.hitbox;
					aabb.min.x -= constants().cellSize,
					spikeCenters.push_back(SpikeCenterInstance{
						.transform = makeObjectTransform(
							aabb.center(),
							0.0f,
							aabb.size() / 2.0f
						) * rot * renderer.camera.worldToCameraToNdc(),
						.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
						.time = backgroundElapsed,
						.normal = normal
					});
				};

			const auto type = roomBlockGrid(xi, yi);

			if (type == BlockType::SPIKE_LEFT) {
				addSpikeCenter(0.0f, Vec2(-1.0f, 0.0f));
			} else if (type == BlockType::SPIKE_TOP) {
				addSpikeCenter(-PI<f32> / 2.0f, Vec2(0.0f, 1.0f));
			} else if (type == BlockType::SPIKE_RIGHT) {
				addSpikeCenter(PI<f32>, Vec2(1.0f, 0.0f));
			}

			if (type == BlockType::SPIKE_BOTTOM) {
				spikeClosedCorner.push_back(SpikeClosedCornerInstance{
					.transform = renderer.camera.makeTransform(
						center,
						0.0f,
						Vec2(constants().cellSize) / 2.0f
					),
					/*.transform = renderer.camera.makeTransform(
						center + Vec2(constants().cellSize * 0.1f),
						0.0f,
						Vec2(constants().cellSize * 1.2f) / 2.0f
					),*/
					.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
					.time = backgroundElapsed,
				});
			}



			//switch (roomBlockGrid(xi, yi)) {
			//case BlockType::SPIKE_LEFT: 
			//{
			//	const auto spike = makeSpikeLeft(xi, yi, roomPosition);
			//	auto aabb = spike.hitbox;
			//	aabb.min.x -= constants().cellSize,
			//	spikeCenters.push_back(SpikeCenterInstance{
			//		.transform = renderer.camera.makeTransform(
			//			aabb.center(),
			//			0.0f,
			//			aabb.size() / 2.0f
			//		),
			//		.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
			//		.time = backgroundElapsed,
			//		.normal = Vec2(1.0f, 0.0f)
			//	});
			//	break;
			//}

			//case BlockType::SPIKE_TOP:
			//{
			//	const auto pos = Vec2(xi + roomPosition.x, yi + roomPosition.y) * constants().cellSize;
			//	auto aabb = Aabb(pos - Vec2(constants().cellSize * 0.2f, 0.0f), pos + Vec2(constants().cellSize) + Vec2(0.0f, constants().cellSize * 0.2f));
			//	spikeOpenCorner.push_back(SpikeOpenCornerInstance{
			//		.transform = renderer.camera.makeTransform(
			//			aabb.center(),
			//			0.0f,
			//			aabb.size() / 2.0f
			//		),
			//		.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
			//		.time = backgroundElapsed
			//	});
			//	break;
			//}

			//case BlockType::SPIKE_RIGHT:
			//{
			///*	const auto rot = 
			//		Mat3x2::translate(-center) *
			//		Mat3x2::rotate(-PI<f32> / 2.0f) *
			//		Mat3x2::translate(-center);*/
			//	/*const auto rot =
			//		Mat3x2::translate(center) *
			//		Mat3x2::rotate(-PI<f32> / 2.0f) *
			//		Mat3x2::translate(-center);*/
			//	const auto rot =
			//		Mat3x2::translate(-center) *
			//		Mat3x2::rotate(-PI<f32> / 2.0f) *
			//		Mat3x2::translate(center);
			//	/*const auto rot =
			//		Mat3x2::translate(Vec2(1.0f, 0.0f));*/
 		//		const auto spike = makeSpikeLeft(xi, yi, roomPosition);
			//	auto aabb = spike.hitbox;
			//	aabb.min.x -= constants().cellSize,
			//	spikeCenters.push_back(SpikeCenterInstance{
			//		.transform = makeObjectTransform(
			//			aabb.center(),
			//			0.0f,
			//			aabb.size() / 2.0f
			//		) * rot * renderer.camera.worldToCameraToNdc(),
			//		.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
			//		.time = backgroundElapsed,
			//		.normal = Vec2(0.0f, 1.0f)
			//	});
			//	break;
			//}

			//
			//case BlockType::SPIKE_BOTTOM: 
			//{
			//	spikeClosedCorner.push_back(SpikeClosedCornerInstance{
			//		.transform = renderer.camera.makeTransform(
			//			center,
			//			0.0f,
			//			Vec2(constants().cellSize) / 2.0f
			//		),
			//		.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
			//		.time = backgroundElapsed,
			//	});
			//	break;
			//}


			//}
			
		}
	}

	glEnable(GL_BLEND);
	spikeCenterShader.use();
	drawInstances(spikeCenterVao, renderer.instancesVbo, spikeCenters, [](usize instanceCount) {
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, instanceCount);
	});

	spikeOpenCornerShader.use();
	drawInstances(spikeOpenCornerVao, renderer.instancesVbo, spikeOpenCorner, [](usize instanceCount) {
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, instanceCount);
	});

	spikeClosedCornerShader.use();
	drawInstances(spikeClosedCornerVao, renderer.instancesVbo, spikeClosedCorner, [](usize instanceCount) {
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, instanceCount);
	});

	glDisable(GL_BLEND);
}

void GameRenderer::renderSpike(const Spike& spike) {
	Dbg::drawFilledAabb(
		spike.hitbox.min,
		spike.hitbox.max,
		Color3::RED);
}

void GameRenderer::renderPlatform(const Platform& platform) {
	Dbg::drawLine(platform.position, platform.position + Vec2(constants().cellSize, 0.0f), BLOCK_COLOR, 2.0f);
}

void GameRenderer::renderPlayer(const Player& player) {
	Dbg::drawFilledAabb(
		player.position - constants().playerSize / 2.0f,
		player.position + constants().playerSize / 2.0f,
		Color3::WHITE);
}

void GameRenderer::renderPlayerFull(const Player& player) {
	PlayerInstance instance{
		.transform = renderer.camera.makeTransform(
			player.position, 
			0.0f, 
			Vec2(constants().playerSize.y) * 4.0f
		),
		.time = backgroundElapsed
	};
	std::vector<PlayerInstance> instances;
	instances.push_back(instance);
	playerShader.use();
	drawInstances(playerVao, renderer.instancesVbo, instances, [](usize instanceCount) {
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, instanceCount);
	});
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

void GameRenderer::renderBackground() {
	backgroundShader.use();
	std::vector<BackgroundInstance> instances;
	instances.push_back(BackgroundInstance{
		.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
		.cameraPosition = renderer.camera.pos,
		.time = backgroundElapsed,
	});
	drawInstances(backgroundVao, renderer.instancesVbo, instances, [](usize count) {
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count);
	});
}
