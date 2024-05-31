#pragma once

#include <platformer/GameRenderer.hpp>
#include <platformer/Editor.hpp>
#include <platformer/GameRoom.hpp>
#include <RefOptional.hpp> 
#include <Array2d.hpp> 
#include <platformer/GameAudio.hpp>
#include <platformer/Menu.hpp>

struct Game {
	Game(GameAudio& audio, GameRenderer& renderer);
	GameAudio& audio;
	GameRenderer& renderer;

	enum class Event {
		PREFORM_GAME_SAVE,
		PAUSE,
	};
	std::vector<Event> thisFrameEvents;

	void onTransitionFromMenu(const GameSave& save);
	void onPause();
	void onUnpause();

	void update(const GameInput& input, const SettingsControls& controlsSettings);
	void gameUpdate(const GameInput& input);
	void gameRender(const SettingsControls& controlsSettings);
	void updateCamera();

	void spawnPlayer(std::optional<i32> editorSelectedRoomIndex);
	void respawnPlayer();

	enum State {
		ALIVE,
		RESPAWNING,
	};
	static constexpr auto respawnAnimationLength = 1.0f;
	f32 respawnElapsed = 0.0f;
	f32 respawnT() const;
	bool respawningUpdateGame() const;
	State state = State::ALIVE;

	void respawningUpdate();
	void respawningRender();

	Player player;

	f32 dt = 1.0f / 60.0f;

	void onSwitchFromEditor(std::optional<i32> editorSelectedRoomIndex);
	void onSwitchToEditor();

	void loadLevel(std::string_view path, std::optional<i32> roomIndex);
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
};