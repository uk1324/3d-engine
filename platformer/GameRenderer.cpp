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
#include <platformer/Paths.hpp>

#define MAKE_VAO(Type) \
	createInstancingVao<Type##Shader>(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo)

GameRenderer::GameRenderer()
	: renderer(Renderer2d::make())
	, fontRenderer(renderer.fullscreenQuad2dPtVerticesVbo, renderer.fullscreenQuad2dPtVerticesIbo, renderer.instancesVbo)
	// TODO: maybe allow the manager to take one source one path to source.
	, gridShader(MAKE_GENERATED_SHADER(GRID))
	, gridVao(MAKE_VAO(Grid))
	, backgroundShader(MAKE_GENERATED_SHADER(BACKGROUND))
	, backgroundVao(MAKE_VAO(Background))
	, blocksShader(MAKE_GENERATED_SHADER(BLOCKS))
	, playerVao(MAKE_VAO(Player))
	, playerShader(MAKE_GENERATED_SHADER(PLAYER))
	, spikeCenterVao(MAKE_VAO(SpikeCenter))
	, spikeCenterShader(MAKE_GENERATED_SHADER(SPIKE_CENTER))
	, spikeOpenCornerVao(MAKE_VAO(SpikeOpenCorner))
	, spikeOpenCornerShader(MAKE_GENERATED_SHADER(SPIKE_OPEN_CORNER))
	, spikeClosedCornerVao(MAKE_VAO(SpikeClosedCorner))
	, spikeClosedCornerShader(MAKE_GENERATED_SHADER(SPIKE_CLOSED_CORNER))
	, attractingOrbVao(MAKE_VAO(AttractingOrb))
	, attractingOrbShader(MAKE_GENERATED_SHADER(ATTRACTING_ORB)) 
	, doubleJumpOrbVao(MAKE_VAO(DoubleJumpOrb))
	, doubleJumpOrbShader(MAKE_GENERATED_SHADER(DOUBLE_JUMP_ORB))
	, font(FontRenderer::loadFont(ASSETS_PATH "fonts", "Orbitron-Medium")) {

}

void GameRenderer::update() {
	renderer.update();
}

void GameRenderer::updateAnimatitons() {
	backgroundElapsed += 1.0f / 60.0f * 2.0f; // * 2, because I accidentally put 2 update calls in the render function (before this code was in update) and all animations relly on this being at this speed. TODO: Fix.
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
void GameRenderer::renderBlockOutline(Vec2 min, Vec2 max) {
	const auto cellBottomLeft = min;
	const auto cellTopRight = max;
	float b = 2.0f;
	const auto size = max - min;
	const auto color = Color3::WHITE / 5.0f;
	Dbg::drawFilledAabb(cellBottomLeft, cellBottomLeft + Vec2(size.x, b), color);
	Dbg::drawFilledAabb(cellTopRight - Vec2(size.x, b), cellTopRight, color);
	Dbg::drawFilledAabb(cellBottomLeft, cellBottomLeft + Vec2(b, size.y), color);
	Dbg::drawFilledAabb(cellTopRight - Vec2(b, size.y), cellTopRight, color);
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

			auto rotateAround = [](Vec2 point, f32 angle) -> Mat3x2 {
				return Mat3x2::translate(-point)*
					Mat3x2::rotate(angle)*
					Mat3x2::translate(point);
			};

			auto reflectAround = [](Vec2 point) -> Mat3x2 {
				return Mat3x2::translate(-point) *
					Mat3x2::scale(Vec2(-1.0f, 1.0f)) *
					Mat3x2::translate(point);
			};

			auto addSpikeCenter = [&](f32 rotation, Vec2 normal) {
				const auto spike = makeSpikeLeft(xi, yi, roomPosition);

				const Vec2 tangent = Vec2(-normal.y, normal.x);

				auto aabb = spike.hitbox;
				aabb.min.x -= constants().cellSize;

				// The second open corner spike is a hacky way to extend the glow so that it doesn't abruptly stop.
				f32 off = 0.3f;
				if (roomBlockGrid.isInBounds(xi - tangent.x, yi - tangent.y) &&
					roomBlockGrid(xi - tangent.x, yi - tangent.y) == BlockType::EMPTY) {
					aabb.max.y -= constants().cellSize * off;

					f32 r = rotation + PI<f32> / 2.0f;
					auto pos = Vec2(xi + roomPosition.x + 1.0f - off, yi + roomPosition.y) * constants().cellSize;
					auto aabb = Aabb(pos, pos + Vec2(constants().cellSize * 1.2f));
					spikeOpenCorner.push_back(SpikeOpenCornerInstance{
						.transform = 
							makeObjectTransform(aabb.center(), 0.0f, aabb.size() / 2.0f) * 
							rotateAround(center, r) * 
							renderer.camera.worldToCameraToNdc(),
						.rotation = -r,
					});

					spikeOpenCorner.push_back(SpikeOpenCornerInstance{
						.transform = 
							makeObjectTransform(aabb.center(), 0.0f, aabb.size() / 2.0f) * 
							rotateAround(pos, -PI<f32> / 2.0f) *
							rotateAround(center, r) * 
							renderer.camera.worldToCameraToNdc(),
						.rotation = -r + PI<f32> / 2.0f,
					});
				}
				if (roomBlockGrid.isInBounds(xi + tangent.x, yi + tangent.y) &&
					roomBlockGrid(xi + tangent.x, yi + tangent.y) == BlockType::EMPTY) {
					aabb.min.y += constants().cellSize * off;

					f32 r = rotation + PI<f32> / 2.0f;
					auto pos = Vec2(xi + roomPosition.x + off, yi + roomPosition.y) * constants().cellSize;
					auto aabb = Aabb(pos, pos + Vec2(constants().cellSize * 1.2f));

					spikeOpenCorner.push_back(SpikeOpenCornerInstance{
						.transform = 
							makeObjectTransform(aabb.center(), 0.0f, aabb.size() / 2.0f) * 
							rotateAround(pos, PI<f32> / 2.0f) *
							rotateAround(center, r) *
							renderer.camera.worldToCameraToNdc(),
						.rotation = -r - PI<f32> / 2.0f,
					});

					spikeOpenCorner.push_back(SpikeOpenCornerInstance{
						.transform = 
							makeObjectTransform(aabb.center(), 0.0f, aabb.size() / 2.0f) * 
							rotateAround(pos, PI<f32>) *
							rotateAround(center, r) *
							renderer.camera.worldToCameraToNdc(),
						.rotation = -r - PI<f32>,
					});
				}

				spikeCenters.push_back(SpikeCenterInstance{
					.transform = makeObjectTransform(
						aabb.center(),
						0.0f,
						aabb.size() / 2.0f
					) * rotateAround(center, rotation) * renderer.camera.worldToCameraToNdc(),
					.normal = normal,
				});
			};

			auto addSpikeClosedCorner = [&](f32 rotation) {
				spikeClosedCorner.push_back(SpikeClosedCornerInstance{
					.transform = makeObjectTransform(
						center,
						0.0f,
						Vec2(constants().cellSize) / 2.0f
					) * rotateAround(center, rotation) * renderer.camera.worldToCameraToNdc(),
					.rotation = -rotation,
				});
			};

			auto addSpikeOpenCorner = [&](f32 rotation) {
				const auto pos = Vec2(xi + roomPosition.x, yi + roomPosition.y) * constants().cellSize;
				auto aabb = Aabb(pos, pos + Vec2(constants().cellSize * 1.2f));
				spikeOpenCorner.push_back(SpikeOpenCornerInstance{
					.transform = makeObjectTransform(
						aabb.center(),
						0.0f,
						aabb.size() / 2.0f
					) * rotateAround(center, rotation) * renderer.camera.worldToCameraToNdc(),
					.rotation = -rotation,
				});
			};

			const auto type = roomBlockGrid(xi, yi);
			if (type == BlockType::SPIKE_LEFT) {
				addSpikeCenter(0.0f, Vec2(-1.0f, 0.0f));
			} else if (type == BlockType::SPIKE_TOP) {
				addSpikeCenter(-PI<f32> / 2.0f, Vec2(0.0f, 1.0f));
			} else if (type == BlockType::SPIKE_RIGHT) {
				addSpikeCenter(PI<f32>, Vec2(1.0f, 0.0f));
			} else if (type == BlockType::SPIKE_BOTTOM) {
				addSpikeCenter(PI<f32> / 2.0f, Vec2(0.0f, -1.0f));
			}

			if (type == BlockType::SPIKE_TOP_RIGHT_CLOSED) {
				addSpikeClosedCorner(PI<f32>);
			}

			if (type == BlockType::SPIKE_TOP_LEFT_CLOSED) {
				addSpikeClosedCorner(-PI<f32> / 2.0f);
			}

			if (type == BlockType::SPIKE_BOTTOM_LEFT_CLOSED) {
				addSpikeClosedCorner(0.0f);
			}

			if (type == BlockType::SPIKE_BOTTOM_RIGHT_CLOSED) {
				addSpikeClosedCorner(PI<f32> / 2.0f);
			}

			if (type == BlockType::SPIKE_TOP_RIGHT_OPEN) {
				addSpikeOpenCorner(0.0f);
			}
			if (type == BlockType::SPIKE_TOP_LEFT_OPEN) {
				addSpikeOpenCorner(PI<f32> / 2.0f);
			}
			if (type == BlockType::SPIKE_BOTTOM_RIGHT_OPEN) {
				addSpikeOpenCorner(-PI<f32> / 2.0f);
			}
			if (type == BlockType::SPIKE_BOTTOM_LEFT_OPEN) {
				addSpikeOpenCorner(PI<f32>);
			}
			
		}
	}

	glEnable(GL_BLEND);

	const auto clipToWorld = renderer.camera.clipSpaceToWorldSpace();
	const auto time = backgroundElapsed;

	{
		spikeCenterShader.use();
		shaderSetUniforms(spikeCenterShader, SpikeCenterVertUniforms{
			.clipToWorld = clipToWorld
		});
		shaderSetUniforms(spikeCenterShader, SpikeCenterFragUniforms{
			.time = time
		});
		drawInstances(spikeCenterVao, renderer.instancesVbo, spikeCenters, Renderer2d::drawFullscreenQuad2dInstances);
	}

	{
		spikeOpenCornerShader.use();
		shaderSetUniforms(spikeOpenCornerShader, SpikeOpenCornerVertUniforms{
			.clipToWorld = clipToWorld
		});
		shaderSetUniforms(spikeOpenCornerShader, SpikeOpenCornerFragUniforms{
			.time = time
		});
		drawInstances(spikeOpenCornerVao, renderer.instancesVbo, spikeOpenCorner, Renderer2d::drawFullscreenQuad2dInstances);
	}

	{
		spikeClosedCornerShader.use();
		shaderSetUniforms(spikeClosedCornerShader, SpikeClosedCornerVertUniforms{
			.clipToWorld = clipToWorld
		});
		shaderSetUniforms(spikeClosedCornerShader, SpikeClosedCornerFragUniforms{
			.time = time
		});
		drawInstances(spikeClosedCornerVao, renderer.instancesVbo, spikeClosedCorner, Renderer2d::drawFullscreenQuad2dInstances);
	}

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
	drawInstances(playerVao, renderer.instancesVbo, instances, Renderer2d::drawFullscreenQuad2dInstances);
}

void GameRenderer::renderDoubleJumpOrb(const Vec2 position) {
	Dbg::drawCircle(position, constants().doubleJumpOrbRadius, Color3::GREEN, 0.1f);
}

void GameRenderer::renderAttractingOrb(const Vec2 position) {
	Dbg::drawCircle(position, constants().attractingOrbRadius, Color3::CYAN, 0.1f);
}

// https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
void replaceAll(std::string& str, const std::string_view& from, const std::string_view& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

#include <engine/Input/InputUtils.hpp>

std::string GameRenderer::processText(std::string_view text, const SettingsControls& controlsSettings) {
	auto out = std::string(text);
	
	replaceAll(out, "`jumpKey`", toString(static_cast<KeyCode>(controlsSettings.jump)));
	replaceAll(out, "`activateKey`", toString(static_cast<KeyCode>(controlsSettings.activate)));
	return out;
	/*std::string out;
	i32 currentCharIndex = 0;

	auto isAtEnd = [&]() {
		return currentCharIndex < text.size();
	};
	auto currentChar = [&]() {
		return text[currentCharIndex];
	};
	for (;;) {
		

		if (currentChar() == '`') {
			while (!isAtEnd() && currentChar() != '`') {

				currentCharIndex++;
			}
		} else {
			out += 
			currentChar++;
		}

		if (isAtEnd()) {
			break;
		}
	}*/

	return out;
}

const auto textSizeY = 20.0f;

void GameRenderer::addText(std::string_view text, Vec2 positionInRoom, Vec2 roomOffset) {
	const auto info = fontRenderer.getTextInfo(font, textSizeY, text);
	Vec2 position = positionInRoom + roomOffset;
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	
	i32 startIndex = fontRenderer.basicTextInstances.size();
	fontRenderer.addTextToDraw(
		font,
		position,
		renderer.camera.worldToCameraToNdc(),
		textSizeY,
		text
	);
	for (i32 i = startIndex; i < fontRenderer.basicTextInstances.size(); i++) {
		fontRenderer.basicTextInstances[i].offset = 0.0f;
	}
}

Aabb GameRenderer::textAabb(std::string_view text, Vec2 positionInRoom, Vec2 roomOffset) const {
	const auto info = fontRenderer.getTextInfo(font, textSizeY, text);
	Vec2 position = positionInRoom + roomOffset;
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	return Aabb(position, position + info.size);
}

void GameRenderer::renderText() {
	glEnable(GL_BLEND);
	fontRenderer.render(font, renderer.instancesVbo);
	glDisable(GL_BLEND);
}

void GameRenderer::addAttractingOrbs(const std::vector<AttractingOrb>& attractingOrbs, Vec2 playerPos) {
	for (const auto& orb : attractingOrbs) {
		attractingOrbInstances.push_back(AttractingOrbInstance{
			.transform = renderer.camera.makeTransform(orb.position, 0.0f, Vec2(100.0f)),
			.playerWorldPos = playerPos,
			.orbWorldPos = orb.position,
			.t = orb.animationT
		});
	}
}

void GameRenderer::renderAttractingOrbs() {
	glEnable(GL_BLEND);
	shaderSetUniforms(attractingOrbShader, AttractingOrbFragUniforms{
		.time = backgroundElapsed
	});
	shaderSetUniforms(attractingOrbShader, AttractingOrbVertUniforms{
		.clipToWorld = renderer.camera.clipSpaceToWorldSpace(),
	});
	attractingOrbShader.use();
	drawInstances(
		attractingOrbVao, 
		renderer.instancesVbo, 
		attractingOrbInstances, 
		Renderer2d::drawFullscreenQuad2dInstances);
	glDisable(GL_BLEND);
	attractingOrbInstances.clear();
}

void GameRenderer::addDoubleJumpOrbs(const std::vector<DoubleJumpOrb>& doubleJumpOrbs) {
	for (const auto& orb : doubleJumpOrbs) {
		doubleJumpOrbInstances.push_back(DoubleJumpOrbInstance{
			.transform = renderer.camera.makeTransform(orb.position, 0.0f, Vec2(constants().doubleJumpOrbRadius * 5.0f)),
			.t = orb.animationT(),
			.orbWorldPosition = orb.position
		});
	}
}

void GameRenderer::renderDoubleJumpOrbs() {
	glEnable(GL_BLEND);
	doubleJumpOrbShader.use();
	shaderSetUniforms(doubleJumpOrbShader, DoubleJumpOrbFragUniforms{
		.time = backgroundElapsed,
		.cameraPosition = renderer.camera.pos,
	});
	drawInstances(
		doubleJumpOrbVao, 
		renderer.instancesVbo, 
		doubleJumpOrbInstances, 
		Renderer2d::drawFullscreenQuad2dInstances);
	glDisable(GL_BLEND);
	doubleJumpOrbInstances.clear();
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