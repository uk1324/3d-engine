#pragma once

#include <platformer/GameRenderer.hpp>
#include <platformer/Level.hpp>

struct Editor {
	Editor();

	void update(f32 dt, f32 cellSize);
	void render(GameRenderer& renderer, f32 cellSize, const PlayerSettings& playerSettings);

	void onSwitchToGame();
	void onSwitchFromGame();

	void loadNewLevel(Level&& level, std::string&& newLevelName);

	std::string errorModalMessage;
	const char* errorModalName = "error";
	void openErrorModal();
	void errorModal();

	enum class PlacableItem {
		NORMAL_BLOCK,
		LEVEL_TRANSITON,
		SPIKE_TOP,
		SPIKE_BOTTOM,
		SPIKE_LEFT,
		SPIKE_RIGHT,
		PLATFORM
	};

	struct LevelTransitionPlaceState {
		std::optional<Vec2> triggerCorner0;
		std::optional<Vec2> triggerCorner1;

		std::optional<LevelTransition> onLeftClick(Vec2 cursorPos, f32 cellSize);
		void onRightClick();
	} lavelTransitionPlaceState;

	struct CreateNewLevelInputState {
		std::string name;
		Vec2T<i32> gridSize;

		void openModal();
		struct Result {
			std::string name;
			Level level;
		};
		std::optional<Result> modal();
	} createNewLevelInputState;

	struct OpenLevelModalState {
		std::string name;

		void openModal();
		struct Result {
			std::string name;
			Level level;
		};
		std::optional<Result> modal();
	} openLevelModalState;

	std::optional<std::string> lastLoadedLevel;

	PlacableItem selectedPlacableItem = PlacableItem::NORMAL_BLOCK;

	std::optional<Vec2> moveGrabStartWorldPos;
	Level level;

	Camera camera;
};