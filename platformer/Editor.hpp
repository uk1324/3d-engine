#pragma once

#include <platformer/GameRenderer.hpp>
#include <platformer/Level.hpp>
#include <platformer/EntityArray.hpp>

struct Editor {
	Editor();
	~Editor();
	
	enum class PlacableItem {
		NORMAL_BLOCK,
		SPAWN_POINT,
		SPIKE_TOP,
		SPIKE_BOTTOM,
		SPIKE_LEFT,
		SPIKE_RIGHT,
		PLATFORM,
		DOUBLE_JUMP_ORB,
		MOVING_BLOCK
	};

	// Remember to update the entity arrays.
	using MovingBlockId = EntityArray<LevelMovingBlock>::Id;
	struct EditorRoom {
		Vec2T<i32> position;
		Array2d<BlockType> blockGrid;
		std::vector<LevelSpawnPoint> spawnPoints;
		std::vector<LevelDoubleJumpOrb> doubleJumpOrbs;
		std::vector<MovingBlockId> movingBlocks;
	};
	using RoomId = EntityArray<EditorRoom>::Id;
	/*
	In the editor everything is dynamically changing and I think using an entity system would make things easier, becuase it doesn't things like managing ids and removal. Without it if for example I wanted to remove a room then I would also need to remember to update the references in each place or add an extra level of indirection and update that.
	If some level are static like block in the game loop then it probably doesn't make sense to try to manage them using ids. 
	*/

	void update(f32 dt);
	void updateSelectedRoom();
	void render(GameRenderer& renderer);
	void renderRoom(const EditorRoom& room, GameRenderer& renderer);
	i32 increaseOrDecreaseSize = 1;
	void roomSizeGui(EditorRoom& room);
	void moveObjects(EditorRoom& room, Vec2T<i32> change);

	static Vec2T<i32> worldPositionToWorldGridPosition(Vec2 worldPosition);
	static Vec2 worldPositionRoundedToGrid(Vec2 worldPosition);

	struct OnSwitchToGameResult {
		std::optional<i32> selectedRoomIndex;
	};
	OnSwitchToGameResult onSwitchToGame();
	void onSwitchFromGame();

	void loadNewLevel(Level&& level, std::string&& newLevelName);
	void editorTryLoadLevelFromFile(std::string_view path);

	void saveSettings();

	std::string errorModalMessage;
	const char* errorModalName = "error";
	void openErrorModal();
	void errorModal();

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

	struct MovingBlockPlaceState {
		std::optional<Vec2> blockCorner0;
		std::optional<Vec2> blockCorner1;

		std::optional<LevelMovingBlock> onLeftClick(Vec2 globalCursorPos, Vec2T<i32> roomPosition);
		void onRightClick();
		void render(Vec2 globalCursorPos);
	} movingBlockPlaceState;
	std::optional<MovingBlockId> selectedMovingBlock;

	std::optional<Vec2> moveGrabStartWorldPos;
	std::optional<RoomId> selectedRoomId;

	std::optional<std::string> lastLoadedLevel;

	PlacableItem selectedPlacableItem = PlacableItem::NORMAL_BLOCK;

	EntityArray<LevelMovingBlock> movingBlocks;
	EntityArray<EditorRoom> rooms;

	void loadLevel(Level&& level);
	Level generateLevel();
	struct GenerateLevel2Result {
		Level level;
		std::optional<i32> selectedRoomIndex;
	};
	GenerateLevel2Result generateLevel2();

	Camera camera;
};