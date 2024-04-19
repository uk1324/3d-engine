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
		gameUpdate();
		gameRender();
	} else if (mode == Mode::EDITOR) {
		editor.update(dt, cellSize, playerSettings);
		editor.render(renderer, cellSize, playerSettings);
	}
}

void Game::gameUpdate() {
	std::optional<LevelRoom&> roomWithBiggestOverlap;
	f32 biggestOverlap = 0.0f;
	for (auto& room : level.rooms) {
		const auto aabbRoom = roomAabb(room, cellSize);
		const auto aabbPlayer = player.aabb(playerSettings);
		const auto overlap = aabbPlayer.intersection(aabbRoom);
		const auto overlapArea = overlap.area();
		if (overlapArea > biggestOverlap) {
			biggestOverlap = overlapArea;
			roomWithBiggestOverlap = room;
		}
	}

	if (activeRoom.has_value() && roomWithBiggestOverlap.has_value()) {
		if (&*activeRoom != &*roomWithBiggestOverlap) {
			activeRoom = roomWithBiggestOverlap; 
		}
	}

	player.updateMovement(dt);
	for (const auto room : rooms) {
		
		player.blockCollision(playerSettings, room.blocks, cellSize);

		const auto playerAabb = player.aabb(playerSettings);

		for (const auto& spike : room.spikes) {
			if (!spike.hitbox.collides(playerAabb)) {
				continue;
			}
			if (!activeRoom.has_value()) {
				ASSERT_NOT_REACHED();
				break;
			}
			loadRoom(*activeRoom);
		}

		for (const auto& platform : room.platforms) {
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
	}

}

void Game::gameRender() {
	glClear(GL_COLOR_BUFFER_BIT);
 	updateCamera();

	for (const auto& room : rooms) {
		// TODO: Culling
		renderer.renderer.camera = camera;
		renderer.renderBlocks(room.blocks, cellSize);
		renderer.renderPlayer(player, playerSettings);
		for (const auto& spike : room.spikes) {
			renderer.renderSpike(spike);
		}
		for (const auto& platform : room.platforms) {
			renderer.renderPlatform(platform, cellSize);
		}
		renderer.update();
	}
	
}

void Game::updateCamera() {
	if (!activeRoom.has_value()) {
		return;
	}

	f32 blockWidth = 45 * cellSize;
	f32 blockHeight = 25 * cellSize;
	// ~ 16/9
	camera.changeSizeToFitBox(Vec2(blockWidth, blockHeight));
	//glViewport()

	/*const auto view = Aabb(Vec2(1.0f) * cellSize, Vec2(activeRoom->blockGrid.size() * cellSize));*/
	const auto min = Vec2(activeRoom->position) * cellSize;
	const auto view = Aabb(min, min + Vec2(activeRoom->blockGrid.size() * cellSize));

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

	const auto currentView = camera.aabb();
	if (currentView.size().x > blockWidth) {
		camera.pos.x = (view.max.x - view.min.x) / 2.0f;
	}
	if (currentView.size().y > blockHeight) {
		camera.pos.y = (view.max.y - view.min.y) / 2.0f;
	}
}

void Game::spawnPlayer() {
	if (level.rooms.size() == 0) {
		return;
	}
	auto& room = level.rooms[0];
	if (room.spawnPoints.size() == 0) {
		return;
	}
	auto& spawnPoint = room.spawnPoints[0];
	const auto spawnPointPosition = spawnPointToPlayerSpawnPos(
		room.spawnPoints[0], 
		playerSettings, 
		room.position, 
		cellSize);
	player = Player{};
	player.position = spawnPointPosition;
	activeRoom = room;
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
	rooms.clear();
	for (auto& room : this->level.rooms) {
		loadRoom(room);
	}
 	spawnPlayer();
	/*if (this->level.rooms.size() > 0) {
		loadRoom(this->level.rooms[0]);
	}*/
}

void Game::loadRoom(LevelRoom& room) {
	RuntimeRoom runtimeRoom;
	//activeRoom = room;
	runtimeRoom.blocks.clear();
	runtimeRoom.spikes.clear();
	runtimeRoom.platforms.clear();
	const auto roomOffset = Vec2(room.position) * cellSize;
	for (i32 y = 0; y < room.blockGrid.size().y; y++) {
		for (i32 x = 0; x < room.blockGrid.size().x; x++) {
			using enum BlockType;
			switch (room.blockGrid(x, y)) {
			case NORMAL: {
				const auto collisionDirections = getBlockCollisionDirections(room.blockGrid, x, y);
				runtimeRoom.blocks.push_back(Block{
					.position = Vec2(x * cellSize, y * cellSize) + roomOffset,
					.collisionDirections = collisionDirections
				});
				break;
			}

			case SPIKE_BOTTOM: 
				runtimeRoom.spikes.push_back(makeSpikeBottom(x, y, cellSize, room.position));
				break;
			case SPIKE_LEFT: 
				runtimeRoom.spikes.push_back(makeSpikeLeft(x, y, cellSize, room.position));
				break;
			case SPIKE_RIGHT: 
				runtimeRoom.spikes.push_back(makeSpikeRight(x, y, cellSize, room.position));
				break;
			case SPIKE_TOP: 
				runtimeRoom.spikes.push_back(makeSpikeTop(x, y, cellSize, room.position));
				break;

			case PLATFORM:
				runtimeRoom.platforms.push_back(makePlatform(x, y, cellSize, room.position));
				break;

			case EMPTY: break;
			}
		}
	}

	// TODO: make this a reference
	//std::optional<SpawnPoint> spawnPoint;

	//for (const auto& transition : level.levelTransitions) {
	//	if ((enteredFromLevelName.has_value() && transition.level == *enteredFromLevelName) ||
	//		transition.level == FIRST_LEVEL_NAME) {
	//		spawnPoint = transition;
	//	}
	//	
	//}

	//if (!spawnPoint.has_value()) {
	//	// TODO: Maybe default.
	//	ASSERT_NOT_REACHED();
	//	return;
	//}

	//Vec2 spawnPoint = Vec2(0.0f);
	//if (room.spawnPoints.size() > 0) {
	//	spawnPoint = spawnPointToPlayerSpawnPos(room.spawnPoints[0], playerSettings, room.position, cellSize);
	//}

	///*player = Player{};
	//player.position = levelTransitionToPlayerSpawnPos(*spawnPoint, playerSettings);*/
	//player = Player{};
	//player.position = spawnPoint;
	rooms.push_back(std::move(runtimeRoom));
}
