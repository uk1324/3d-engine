#pragma once

#include <platformer/GameRenderer.hpp>
#include <platformer/Editor.hpp>
#include <RefOptional.hpp>
#include <Array2d.hpp>

struct Game {
	Game();

	void update();
	void gameUpdate();
	void gameRender();
	void updateCamera();

	void spawnPlayer();

	Player player;

	f32 dt = 1.0f / 60.0f;
	f32 cellSize = 20.0f;

	Level level;
	std::optional<LevelRoom&> activeRoom;

	void onSwitchFromEditor();

	void loadRoom(LevelRoom& room);

	std::optional<std::string> enteredFromLevelName;

	PlayerSettings playerSettings;

	enum class Mode {
		EDITOR,
		GAME,
	};

	struct RuntimeRoom {
		std::vector<Block> blocks;
		std::vector<Spike> spikes;
		std::vector<Platform> platforms;
	};
	std::vector<RuntimeRoom> rooms;

	Mode mode = Mode::GAME; 

	Editor editor;

	Camera camera;

	GameRenderer renderer;
};