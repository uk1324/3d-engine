#include <platformer/Game.hpp>
#include <platformer/Constants.hpp>
#include <framework/Dbg.hpp>
#include <framework/ShaderManager.hpp>
#include <platformer/Assets.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Color.hpp>
#include <platformer/Paths.hpp>
#include <engine/Math/Utils.hpp>
#include <glad/glad.h>

Game::Game(GameAudio& audio, GameRenderer& renderer)
	: audio(audio)
	, renderer(renderer) {
	camera.zoom /= 280.0f;
}

void Game::onTransitionFromMenu(const GameSave& save) {
	loadLevel(ASSETS_PATH "levels/level0", save.roomIndex);
}

void Game::onPause() {
	audio.musicStream.pause();
	audio.pauseSoundEffects();
}

void Game::onUnpause() {
	audio.musicStream.play();
	audio.unpauseSoundEffects();
}

void Game::update(const GameInput& input, const SettingsControls& controlsSettings) {
	ShaderManager::update();
	thisFrameEvents.clear();

	if (state == State::ALIVE && Input::isKeyDown(KeyCode::ESCAPE)) {
		thisFrameEvents.push_back(Event::PAUSE);
	}

	#ifdef FINAL_RELEASE
	gameUpdate(input);
	gameRender(controlsSettings);
	#else
	if (Input::isKeyDown(KeyCode::TAB)) {
		if (mode == Mode::EDITOR) {
			const auto result = editor.onSwitchToGame();
			onSwitchFromEditor(result.selectedRoomIndex);
			mode = Mode::GAME;
		} else if (mode == Mode::GAME) {
			editor.onSwitchFromGame();
			onSwitchToEditor();
			mode = Mode::EDITOR;
		}
	}

	if (mode == Mode::GAME) {
		gameUpdate(input);
		gameRender(controlsSettings);
	} else if (mode == Mode::EDITOR) {
		editor.update(dt, renderer);
		editor.render(renderer);
	}
	#endif
}
#include <imgui/imgui.h>
#include <iostream>
void Game::gameUpdate(const GameInput& input) {
	auto attenuate = [this]() {
		auto volume = audio.attractingOrbSource.volume;
		volume *= 0.9f;
		audio.setSoundEffectSourceVolume(audio.attractingOrbSource, volume);
	};
	const auto updateGame = (state == State::ALIVE) || (state == State::RESPAWNING && respawningUpdateGame());

	if (input.use && updateGame) {
		std::optional<f32> maxT;

		if (auto activeRoom = activeGameRoom(); activeRoom.has_value()) {
			for (const auto& orb : activeRoom->attractingOrbs) {
				auto t = smoothstep(0.0f, 1.0f, orb.animationT * 2.0f);
				t *= smoothstep(250.0, 100.0, player.position.distanceTo(orb.position));
				if (!maxT.has_value() || t > maxT) {
					maxT = t;
				}
			}
		}

		if (maxT.has_value()) {
			audio.setSoundEffectSourceVolume(audio.attractingOrbSource, *maxT);
		} else {
			attenuate();
		}
	} else {
		attenuate();
	}

	if (updateGame) {
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
				thisFrameEvents.push_back(Event::PREFORM_GAME_SAVE);
			}
		}

		if (auto activeRoom = activeGameRoom(); activeRoom.has_value()) {
			player.updateVelocity(input, dt, activeRoom->doubleJumpOrbs, activeRoom->attractingOrbs, audio);
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
			for (auto& orb : activeRoom->attractingOrbs) {
				orb.update(input.use, dt);
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
				respawnPlayer();
				return;
			}
		}
	}
	renderer.updateAnimatitons();
	respawningUpdate();
}

#include <platformer/Shaders/blocksData.hpp>
#include <engine/Window.hpp>

void Game::gameRender(const SettingsControls& controlsSettings) {
	glViewport(0, 0, Window::size().x, Window::size().y);
	camera.aspectRatio = Window::aspectRatio();
	glClear(GL_COLOR_BUFFER_BIT);
 	updateCamera();
	renderer.renderer.camera = camera;

	renderer.renderBackground();

	auto isRoomInView = [&](const LevelRoom& room) {
		auto cameraAabb = camera.aabb();
		cameraAabb.min -= Vec2(1.0f);
		cameraAabb.max += Vec2(1.0f);
		const auto roomAabb = ::roomAabb(room);
		return cameraAabb.collides(roomAabb);
	};

	for (i32 i = 0; i < rooms.size(); i++) {
		const auto& room = rooms[i];
		const auto& levelRoom = level.rooms[i];

		if (!isRoomInView(levelRoom)) {
			continue;
		}
		renderer.renderSpikes(levelRoom.blockGrid, levelRoom.position);
		renderer.addAttractingOrbs(room.attractingOrbs, player.position);
		renderer.addDoubleJumpOrbs(room.doubleJumpOrbs);
	}
	renderer.renderAttractingOrbs();
	renderer.renderDoubleJumpOrbs();

	auto renderBlockShader = [&](f32 scale) {
		glEnable(GL_STENCIL_TEST);

		glStencilMask(0xFF);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		//renderer.renderer.drawDbgFilledTriangles();
		renderer.renderer.drawDbgFilledAabbs();

		glStencilMask(0x00);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glStencilFunc(GL_EQUAL, 1, 0xFF);
		{
			renderer.renderer.fullscreenQuad2dPtVerticesVao.bind();
			renderer.blocksShader.use();
			shaderSetUniforms(renderer.blocksShader, BlocksVertUniforms{
				.clipToWorld = renderer.renderer.camera.clipSpaceToWorldSpace(),
			});
			shaderSetUniforms(renderer.blocksShader, BlocksFragUniforms{
				.time = renderer.backgroundElapsed,
				.scale = scale
			});
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		}
		glDisable(GL_STENCIL_TEST);
	};
	
	//renderer.renderer.drawDbgFilledTriangles();
	renderer.renderer.drawDbgFilledAabbs();

	for (i32 i = 0; i < rooms.size(); i++) {
		const auto& room = rooms[i];
		const auto& levelRoom = level.rooms[i];
		if (!isRoomInView(levelRoom)) {
			continue;
		}

		for (const auto& block : room.blocks) {
			Dbg::drawFilledAabb(block.position, block.position + Vec2(constants().cellSize), Color3::WHITE / 20.0f);
		}
	}
	renderBlockShader(1.0f);

	for (i32 i = 0; i < rooms.size(); i++) {
		const auto& room = rooms[i];
		const auto& levelRoom = level.rooms[i];

		auto cameraAabb = camera.aabb();
		cameraAabb.min -= Vec2(1.0f);
		cameraAabb.max += Vec2(1.0f);
		const auto roomAabb = ::roomAabb(levelRoom);

		if (!cameraAabb.collides(roomAabb)) {
			continue;
		}
		renderer.renderBlockOutlines(levelRoom.blockGrid, levelRoom.position);
		for (const auto& platform : room.platforms) {
			renderer.renderPlatform(platform);
		}
		for (const auto& block : room.movingBlocks) {
			const auto position = block.position();
			Dbg::drawFilledAabb(position, position + block.size, Color3::BLACK);
			renderer.renderBlockOutline(block.position(), block.position() + block.size);
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

		for (const auto& text : activeRoom->texts) {
			renderer.addText(renderer.processText(text.text, controlsSettings), text.position, min);
		}
		renderer.renderText();
	}

	{
		glEnable(GL_BLEND);
		renderer.renderPlayerFull(player);
		glDisable(GL_BLEND);
	}

	respawningRender();

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
	for (auto& orb : gameRoom.attractingOrbs) {
		orb.reset();
	}
}

void Game::respawnPlayer() {
	audio.playSoundEffect(assets->playerDeathSound);
	if (state == State::RESPAWNING) {
		ASSERT_NOT_REACHED();
		return;
	}
	state = State::RESPAWNING;
	respawnElapsed = 0.0f;
}

f32 Game::respawnT() const {
	return respawnElapsed / respawnAnimationLength;
}

bool Game::respawningUpdateGame() const {
	return respawnT() > 0.55f;
}

void Game::respawningUpdate() {
	if (state != State::RESPAWNING) {
		return;
	}

	const auto oldT = respawnT();

	respawnElapsed += dt;

	auto t = respawnT();
	if (oldT < 0.5f && t >= 0.5f) {
		t = 0.5f;
		spawnPlayer(std::nullopt);
	}

	if (respawnElapsed >= respawnAnimationLength) {
		state = State::ALIVE;
	}
}

void Game::respawningRender() {
	const auto view = camera.aabb();
	const auto viewSize = view.size();
	auto start = view;
	start.min.y -= viewSize.y;
	start.max.y -= viewSize.y;
	auto end = view;
	end.min.y += viewSize.y;
	end.max.y += viewSize.y;
	auto t = respawnT();
	Dbg::drawFilledAabb(lerp(start.min, end.min, t), lerp(start.max, end.max, t), Color3::BLACK);
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
	loadLevel(*editor.lastLoadedLevel, editorSelectedRoomIndex);
	audio.musicStream.play();
}

void Game::onSwitchToEditor() {
	audio.stopSoundEffects();
	audio.musicStream.pause();
}

void Game::loadLevel(std::string_view path, std::optional<i32> roomIndex) {
	audio.initGameAudio();

	auto level = tryLoadLevelFromFile(path);
	if (!level.has_value()) {
		return;
	}

	this->level = std::move(*level);
	rooms.clear();
	for (auto& room : this->level.rooms) {
		loadRoom(room);
	}
	spawnPlayer(roomIndex);
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

	for (const auto& orb : room.attractingOrbs) {
		runtimeRoom.attractingOrbs.push_back(AttractingOrb{
			.position = orb.position + roomOffset,
		});
	}

	for (const auto& block : room.movingBlocks) {
		runtimeRoom.movingBlocks.push_back(MovingBlock(block, room.position));
	}
	rooms.push_back(std::move(runtimeRoom));
}
