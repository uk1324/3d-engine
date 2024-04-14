#pragma once

#include <platformer/GameRenderer.hpp>
#include <platformer/Editor.hpp>
#include <Array2d.hpp>

struct Game {
	Game();

	void update();
	void updateCamera();

	Player player;

	f32 dt = 1.0f / 60.0f;
	f32 cellSize = 20.0f;

	Level level;

	void onSwitchFromEditor();

	void loadLevel(const Level& level);

	std::optional<std::string> enteredFromLevelName;

	//Array2d<BlockType> blockGrid;

	PlayerSettings playerSettings;

	enum class Mode {
		EDITOR,
		GAME,
	};

	std::vector<Block> blocks;

	Mode mode = Mode::GAME; 

	Editor editor;

	Camera camera;

	GameRenderer renderer;
};