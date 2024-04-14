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

		for (const auto& spike : spikes) {
			if (!spike.hitbox.collides(playerAabb)) {
				continue;
			}
			loadLevel(this->level);
		}

		for (const auto& platform : platforms) {
			const auto hitbox = Aabb(platform.position, platform.position + Vec2(cellSize, 0.0f));
			auto playerHitbox = playerAabb;
			const auto padding = player.velocity.applied(abs);
			playerHitbox.min -= padding;
			playerHitbox.max += padding;

			if (!hitbox.collides(playerHitbox)) {
				continue;
			}

			if (player.velocity.y > 0) {
				continue;
			}
			
			const auto playerBottomY = player.position.y - playerSettings.size.y / 2.0f;
			const auto platformY = platform.position.y;
			if (playerBottomY >= platformY && playerBottomY + player.velocity.y <= platformY) {
				player.velocity.y = 0.0f;
				player.position.y = platformY + playerSettings.size.y / 2.0f;
				player.grounded = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		updateCamera();
		renderer.renderer.camera = camera;
		renderer.renderBlocks(blocks, cellSize);
		renderer.renderPlayer(player, playerSettings);
		for (const auto& spike : spikes) {
			renderer.renderSpike(spike);
		}
		for (const auto& platform : platforms) {
			renderer.renderPlatform(platform, cellSize);
		}
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
	spikes.clear();
	platforms.clear();
	for (i32 y = 0; y < level.blockGrid.size().y; y++) {
		for (i32 x = 0; x < level.blockGrid.size().x; x++) {
			using enum BlockType;
			switch (level.blockGrid(x, y)) {
			case NORMAL: {
				const auto collisionDirections = getBlockCollisionDirections(level.blockGrid, x, y);
				blocks.push_back(Block{
					.position = Vec2(x * cellSize, y * cellSize),
					.collisionDirections = collisionDirections
				});
				break;
			}

			case SPIKE_BOTTOM: 
				spikes.push_back(makeSpikeBottom(x, y, cellSize));
				break;
			case SPIKE_LEFT: 
				spikes.push_back(makeSpikeLeft(x, y, cellSize));
				break;
			case SPIKE_RIGHT: 
				spikes.push_back(makeSpikeRight(x, y, cellSize));
				break;
			case SPIKE_TOP: 
				spikes.push_back(makeSpikeTop(x, y, cellSize));
				break;

			case PLATFORM:
				platforms.push_back(makePlatform(x, y, cellSize));
				break;

			case EMPTY: break;
			}
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
