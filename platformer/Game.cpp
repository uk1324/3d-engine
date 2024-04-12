#include <platformer/Game.hpp>
#include <framework/Dbg.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Color.hpp>
#include <glad/glad.h>

Game::Game() 
	: blockGrid(100, 100) {

	camera.zoom /= 500.0f;

	player.position = Vec2(0.1f, 100.0f);
	player.velocity = Vec2(0.0f);

	playerSettings.size = Vec2(30.0f);

	std::ranges::fill(blockGrid.span(), BlockType::EMPTY);

	blockGrid(0, 0) = BlockType::NORMAL;
	blockGrid(1, 0) = BlockType::NORMAL;
	blockGrid(2, 0) = BlockType::NORMAL;
	blockGrid(2, 1) = BlockType::NORMAL;

	for (i32 y = 0; y < blockGrid.size().y; y++) {
		for (i32 x = 0; x < blockGrid.size().x; x++) {
			if (blockGrid(x, y) != BlockType::NORMAL) {
				continue;
			}

			struct Entry {
				i32 x;
				i32 y;
				BlockCollision::Direction direction;
			};
			using enum BlockCollision::Direction;
			Entry directions[] {
				{ .x = 1, .y = 0, .direction = R },
				{ .x = -1, .y = 0, .direction = L },
				{ .x = 0, .y = 1, .direction = U },
				{ .x = 0, .y = -1, .direction = D },
			};

			u8 collisionDirections = 0b0000;
			for (auto& direction : directions) {
				const i32 xd = x + direction.x;
				const i32 yd = y + direction.y;

				const bool isOutOfRange =
					xd >= blockGrid.size().x ||
					yd >= blockGrid.size().y ||
					xd < 0 ||
					yd < 0;

				if (isOutOfRange || blockGrid(xd, yd) == BlockType::EMPTY) {
					collisionDirections |= direction.direction;
				}
			}

			blocks.push_back(Block{
				.position = Vec2(x * cellSize, y * cellSize),
				.collisionDirections = collisionDirections
			});
		}
	}
}

void Game::update() {

	if (Input::isKeyDown(KeyCode::TAB)) {
		if (mode == Mode::EDITOR) mode = Mode::GAME;
		else if (mode == Mode::GAME) mode = Mode::EDITOR;
	}

	if (mode == Mode::GAME) {
		player.updateMovement(dt);
		player.blockCollision(playerSettings, blocks, cellSize);

		glClear(GL_COLOR_BUFFER_BIT);
		renderer.renderer.camera = camera;
		renderer.renderBlocks(blocks, cellSize);
		renderer.renderPlayer(player, playerSettings);
		renderer.update();
	} else if (mode == Mode::EDITOR) {
		editor.update(dt);
		editor.render(renderer, cellSize);
	}
}
