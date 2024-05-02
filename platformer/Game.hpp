#pragma once

#include <platformer/GameRenderer.hpp>
#include <platformer/Editor.hpp>
#include <platformer/GameRoom.hpp>
#include <RefOptional.hpp>
#include <Array2d.hpp>

struct Game {
	Game();

	void update();
	void gameUpdate();
	void gameRender();
	void updateCamera();

	void spawnPlayer(std::optional<i32> editorSelectedRoomIndex);

	Player player;

	f32 dt = 1.0f / 60.0f;
	//PlayerSettings playerSettings;
	//f32 cellSize = 20.0f;

	void onSwitchFromEditor(std::optional<i32> editorSelectedRoomIndex);

	void loadRoom(LevelRoom& room);

	std::optional<std::string> enteredFromLevelName;

	enum class Mode {
		EDITOR,
		GAME,
	};


	std::vector<GameRoom> rooms;

	Level level;
	std::optional<i32> activeRoomIndex;
	std::optional<LevelRoom&> activeLevelRoom();
	std::optional<GameRoom&> activeGameRoom();

	Mode mode = Mode::GAME; 

	Editor editor;

	Camera camera;

	GameRenderer renderer;
};