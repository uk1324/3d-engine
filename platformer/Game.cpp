#include <platformer/Game.hpp>
#include <framework/Dbg.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Color.hpp>
#include <glad/glad.h>

Game::Game() {
	camera.zoom /= 280.0f;
}

void Game::update() {
	playerSettings.size = Vec2(20.0f, 30.0f);

	if (Input::isKeyDown(KeyCode::TAB)) {
		if (mode == Mode::EDITOR) {
			editor.onSwitchToGame();
			onSwitchFromEditor();
			mode = Mode::GAME;
		}
		else if (mode == Mode::GAME) {
			editor.onSwitchFromGame();
			mode = Mode::EDITOR;
		}
	}

	if (mode == Mode::GAME) {
		player.updateMovement(dt);
		player.blockCollision(playerSettings, blocks, cellSize);

		const auto playerAabb = player.aabb(playerSettings);
		for (const auto& transition : level.levelTransitions) {
			if (!transition.trigger.collides(playerAabb)) {
				continue;
			}

			auto level = tryLoadLevelFromFile(levelsPath + transition.level);
			if (level.has_value()) {
				this->level = std::move(*level);
				loadLevel(this->level);
				break;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		updateCamera();
		renderer.renderer.camera = camera;
		renderer.renderBlocks(blocks, cellSize);
		renderer.renderPlayer(player, playerSettings);
		renderer.update();
	} else if (mode == Mode::EDITOR) {
		editor.update(dt, cellSize);
		editor.render(renderer, cellSize, playerSettings);
	}
}

void Game::updateCamera() {
	const auto view = Aabb(Vec2(1.0f) * cellSize, Vec2(level.blockGrid.size() * cellSize));

	const auto destination = player.position;
      
	camera.pos = destination;

	const auto halfCameraSize = camera.aabb().size() / 2.0f;

	if (camera.pos.x - halfCameraSize.x < view.min.x) {
		camera.pos.x = view.min.x + halfCameraSize.x;
	}
	if (camera.pos.y - halfCameraSize.y < view.min.y) {
		camera.pos.y = view.min.y + halfCameraSize.y;
	}

	if (camera.pos.x > view.max.x - halfCameraSize.x) {
		camera.pos.x = view.max.x - halfCameraSize.x;
	}

	if (camera.pos.y > view.max.y - halfCameraSize.y) {
		camera.pos.y = view.max.y - halfCameraSize.y;
	}
}

void Game::onSwitchFromEditor() {
	if (!editor.lastLoadedLevel.has_value()) {
		return;
	}
	auto level = tryLoadLevelFromFile(*editor.lastLoadedLevel);
	if (!level.has_value()) {
		return;
	}

	this->level = std::move(*level);
	loadLevel(this->level);
}

void Game::loadLevel(const Level& level) {
	blocks.clear();
	for (i32 y = 0; y < level.blockGrid.size().y; y++) {
		for (i32 x = 0; x < level.blockGrid.size().x; x++) {
			if (level.blockGrid(x, y) != BlockType::NORMAL) {
				continue;
			}

			const auto collisionDirections = getBlockCollisionDirections(level.blockGrid, x, y);

			blocks.push_back(Block{
				.position = Vec2(x * cellSize, y * cellSize),
				.collisionDirections = collisionDirections
			});
		}
	}

	// TODO: make this a reference
	std::optional<LevelTransition> spawnPoint;

	for (const auto& transition : level.levelTransitions) {
		if ((enteredFromLevelName.has_value() && transition.level == *enteredFromLevelName) ||
			transition.level == FIRST_LEVEL_NAME) {
			spawnPoint = transition;
		}
		
	}

	if (!spawnPoint.has_value() && level.levelTransitions.size()) {
		spawnPoint = level.levelTransitions[0];
	}

	if (!spawnPoint.has_value()) {
		// TODO: Maybe default.
		ASSERT_NOT_REACHED();
		return;
	}

	player = Player{};
	player.position = levelTransitionToPlayerSpawnPos(*spawnPoint, playerSettings);
}
