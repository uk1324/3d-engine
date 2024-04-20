#include <platformer/Game.hpp>
#include <platformer/Constants.hpp>
#include <framework/Dbg.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Color.hpp>
#include <glad/glad.h>

Game::Game() {
	camera.zoom /= 280.0f;
}

void Game::update() {
	if (Input::isKeyDown(KeyCode::TAB)) {
		if (mode == Mode::EDITOR) {
			const auto result = editor.onSwitchToGame();
			onSwitchFromEditor(result.selectedRoomIndex);
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
		editor.update(dt);
		editor.render(renderer);
	}
}

void Game::gameUpdate() {
	std::optional<i32> roomWithBiggestOverlapIndex;
	f32 biggestOverlap = 0.0f;
	for (i32 i = 0; i < level.rooms.size(); i++) {
		auto& room = level.rooms[i];
		const auto aabbRoom = roomAabb(room);
		const auto aabbPlayer = playerAabb(player.position);
		const auto overlap = aabbPlayer.intersection(aabbRoom);
		const auto overlapArea = overlap.area();
		if (overlapArea > biggestOverlap) {
			biggestOverlap = overlapArea;
			roomWithBiggestOverlapIndex = i;
		}
	}

	if (activeRoomIndex.has_value() && roomWithBiggestOverlapIndex.has_value()) {
		if (activeRoomIndex != roomWithBiggestOverlapIndex) {
			activeRoomIndex = roomWithBiggestOverlapIndex;
		}
	}

	if (auto activeRoom = activeRuntimeRoom(); activeRoom.has_value()) {
		for (auto& orb : activeRoom->doubleJumpOrbs) {
			orb.elapsedSinceUsed += dt;
		}
		player.updateMovement(dt, activeRoom->doubleJumpOrbs);
	}

	for (const auto room : rooms) {
		
		player.blockCollision(room.blocks);

		const auto playerAabb = ::playerAabb(player.position);

		for (const auto& spike : room.spikes) {
			if (!spike.hitbox.collides(playerAabb)) {
				continue;
			}
			spawnPlayer(std::nullopt);
		}

		for (const auto& platform : room.platforms) {
			const auto hitbox = Aabb(platform.position, platform.position + Vec2(constants().cellSize, 0.0f));
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

			const auto playerBottomY = player.position.y - constants().playerSize.y / 2.0f;
			const auto platformY = platform.position.y;
			if (playerBottomY >= platformY && playerBottomY + player.velocity.y <= platformY) {
				player.velocity.y = 0.0f;
				player.position.y = platformY + constants().playerSize.y / 2.0f;
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
		renderer.renderBlocks(room.blocks);
		renderer.renderPlayer(player);
		for (const auto& spike : room.spikes) {
			renderer.renderSpike(spike);
		}
		for (const auto& platform : room.platforms) {
			renderer.renderPlatform(platform);
		}
		for (const auto& orb : room.doubleJumpOrbs) {
			renderer.renderDoubleJumpOrb(orb);
		}
	}
	renderer.update();

	if (const auto activeRoom = activeLevelRoom(); activeRoom.has_value()) {
		const auto min = Vec2(activeRoom->position) * constants().cellSize;
		const auto roomAabb = ::roomAabb(*activeRoom);
		const auto viewAabb = camera.aabb();
		if (roomAabb.size().y < viewAabb.size().y) {
			Dbg::drawFilledAabb(viewAabb.min, Vec2(viewAabb.max.x, roomAabb.min.y), Color3::BLACK);
			Dbg::drawFilledAabb(Vec2(viewAabb.min.x, roomAabb.max.y), viewAabb.max, Color3::BLACK);
		}
	}
	renderer.update();
}

void Game::updateCamera() {
	auto activeRoom = activeLevelRoom();
	if (!activeRoom.has_value()) {
		return;
	}
	
	f32 blockWidth = 45 * constants().cellSize;
	f32 blockHeight = 25 * constants().cellSize;
	// ~ 16/9
	camera.changeSizeToFitBox(Vec2(blockWidth, blockHeight));
	//glViewport()

	/*const auto view = Aabb(Vec2(1.0f) * cellSize, Vec2(activeRoom->blockGrid.size() * cellSize));*/
	const auto view = ::roomAabb(*activeRoom);

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
	if (currentView.size().x > view.size().x) {
		camera.pos.x = (view.max.x + view.min.x) / 2.0f;
	}
	if (currentView.size().y > view.size().y) {
		camera.pos.y = (view.max.y + view.min.y) / 2.0f;
	}
}

void Game::spawnPlayer(std::optional<i32> editorSelectedRoomIndex) {
	if (level.rooms.size() == 0) {
		return;
	}
	i32 roomIndex = 0;
	if (editorSelectedRoomIndex.has_value() && *editorSelectedRoomIndex < level.rooms.size()) {
		roomIndex = *editorSelectedRoomIndex;
	} else if (activeRoomIndex.has_value()) {
		roomIndex = *activeRoomIndex;
	}

	auto& room = level.rooms[roomIndex];
	if (room.spawnPoints.size() == 0) {
		return;
	}
	auto& spawnPoint = room.spawnPoints[0];
	const auto spawnPointPosition = spawnPointToPlayerSpawnPos(
		room.spawnPoints[0], 
		room.position);
	player = Player{};
	player.position = spawnPointPosition;
	activeRoomIndex = roomIndex;
}

std::optional<LevelRoom&> Game::activeLevelRoom() {
	if (activeRoomIndex.has_value() && *activeRoomIndex < level.rooms.size()) {
		return level.rooms[*activeRoomIndex];
	}
	return std::nullopt;
}

std::optional<Game::RuntimeRoom&> Game::activeRuntimeRoom() {
	if (activeRoomIndex.has_value() && *activeRoomIndex < rooms.size()) {
		return rooms[*activeRoomIndex];
	}
	return std::nullopt;
}

void Game::onSwitchFromEditor(std::optional<i32> editorSelectedRoomIndex) {
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
 	spawnPlayer(editorSelectedRoomIndex);
	/*if (this->level.rooms.size() > 0) {
		loadRoom(this->level.rooms[0]);
	}*/
}

void Game::loadRoom(LevelRoom& room) {
	RuntimeRoom runtimeRoom;
	//activeRoom = room;
	/*runtimeRoom.blocks.clear();
	runtimeRoom.spikes.clear();
	runtimeRoom.platforms.clear();
	runtimeRoom.*/

	const auto roomOffset = Vec2(room.position) * constants().cellSize;
	for (i32 y = 0; y < room.blockGrid.size().y; y++) {
		for (i32 x = 0; x < room.blockGrid.size().x; x++) {
			using enum BlockType;
			switch (room.blockGrid(x, y)) {
			case NORMAL: {
				const auto collisionDirections = getBlockCollisionDirections(room.blockGrid, x, y);
				runtimeRoom.blocks.push_back(Block{
					.position = Vec2(x, y) * constants().cellSize + roomOffset,
					.collisionDirections = collisionDirections
				});
				break;
			}

			case SPIKE_BOTTOM: 
				runtimeRoom.spikes.push_back(makeSpikeBottom(x, y, room.position));
				break;
			case SPIKE_LEFT: 
				runtimeRoom.spikes.push_back(makeSpikeLeft(x, y, room.position));
				break;
			case SPIKE_RIGHT: 
				runtimeRoom.spikes.push_back(makeSpikeRight(x, y, room.position));
				break;
			case SPIKE_TOP: 
				runtimeRoom.spikes.push_back(makeSpikeTop(x, y, room.position));
				break;

			case PLATFORM:
				runtimeRoom.platforms.push_back(makePlatform(x, y, room.position));
				break;

			case EMPTY: break;
			}
		}
	}

	for (const auto& orb : room.doubleJumpOrbs) {
		runtimeRoom.doubleJumpOrbs.push_back(DoubleJumpOrb{
			.position = orb.position + roomOffset,
			.elapsedSinceUsed = std::numeric_limits<f32>::infinity(),
		});
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
