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
#include <imgui/imgui.h>
void Game::gameUpdate() {
	{
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
	}

	if (auto activeRoom = activeGameRoom(); activeRoom.has_value()) {
		player.updateVelocity(dt, activeRoom->doubleJumpOrbs);
	}

	// Could use an iterator instead of copying this. Could have an iterator of all the active rooms.
	std::vector<GameRoom*> activeRooms;
	for (i32 i = 0; i < rooms.size(); i++) {
		auto& runtimeRoom = rooms[i];
		const auto& levelRoom = level.rooms[i];
		auto playerAabb = ::playerAabb(player.position);
		const auto movement = player.velocity.applied(abs) + Vec2(constants().cellSize);
		playerAabb.min -= movement;
		playerAabb.max += movement;
		const auto roomAabb = ::roomAabb(levelRoom);

		if (!playerAabb.collides(roomAabb)) {
			continue;
		}
		activeRooms.push_back(&runtimeRoom);
	}
	collisionDetection(dt, activeRooms, player);
	//player.collision(dt, activeBlocks, activePlatforms, activeMovingBlocks);

	if (auto activeRoom = activeGameRoom(); activeRoom.has_value()) {
		for (auto& orb : activeRoom->doubleJumpOrbs) {
			orb.elapsedSinceUsed += dt;
		}
		for (auto& block : activeRoom->movingBlocks) {
			block.update(dt);
		}
	}

	for (const auto& room : rooms) {
		auto playerAabb = ::playerAabb(player.position);
		for (const auto& spike : room.spikes) {
			if (!spike.hitbox.collides(playerAabb)) {
				continue;
			}
			spawnPlayer(std::nullopt);
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
		for (const auto& block : room.movingBlocks) {
			const auto position = block.position();
			Dbg::drawAabb(position, position + block.size, Color3::GREEN, 2.0f);
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

	auto& gameRoom = rooms[roomIndex];
	for (auto& block : gameRoom.movingBlocks) {
		block.reset();
	}
	for (auto& orb : gameRoom.doubleJumpOrbs) {
		orb.reset();
	}
}

std::optional<LevelRoom&> Game::activeLevelRoom() {
	if (activeRoomIndex.has_value() && *activeRoomIndex < level.rooms.size()) {
		return level.rooms[*activeRoomIndex];
	}
	return std::nullopt;
}

std::optional<GameRoom&> Game::activeGameRoom() {
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
}

void Game::loadRoom(LevelRoom& room) {
	GameRoom runtimeRoom;

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

	for (const auto& block : room.movingBlocks) {
		runtimeRoom.movingBlocks.push_back(MovingBlock(block, room.position));
	}
	rooms.push_back(std::move(runtimeRoom));
}
