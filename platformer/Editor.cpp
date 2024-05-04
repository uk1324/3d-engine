#include <platformer/Editor.hpp>
#include <framework/Dbg.hpp>
#include <fstream>
#include <Gui.hpp>
#include <framework/CameraUtils.hpp>
#include <engine/Math/Color.hpp>
#include <imgui/imgui_stdlib.h>
#include <engine/Input/Input.hpp>
#include <StringRefStream.hpp>
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <engine/Math/Circle.hpp>
#include <FileIo.hpp>
#include <filesystem>
#include <platformer/Constants.hpp>
#include <platformer/EditorSettingsData.hpp>
#include <engine/Json/JsonParser.hpp>
#include <engine/Json/JsonPrinter.hpp>

static constexpr auto EDITOR_SETTINGS_PATH = "generated/editorSettings.json";

template<typename T, typename F>
void removeIf(std::vector<T>& vs, F f) {
	vs.erase(std::remove_if(
		vs.begin(),
		vs.end(),
		f
	), vs.end());
}

Editor::Editor() {
	auto loadSettings = []() -> std::optional<EditorSettings> {
		const auto text = tryLoadStringFromFile(EDITOR_SETTINGS_PATH);
		if (!text.has_value()) {
			return std::nullopt;
		}
		try {
			const auto json = Json::parse(*text);
			return fromJson<EditorSettings>(json);
		}
		catch (const Json::Value::Exception&) {
			return std::nullopt;
		}
		catch (const Json::ParsingError&) {
			return std::nullopt;
		}
	};

	const auto settings = loadSettings();
	if (settings.has_value()) {
		lastLoadedLevel = settings->lastLoadedLevel;
		camera.pos = settings->lastLoadedCameraPosition;
		camera.zoom = settings->lastLoadedCameraZoom;
	}
	 
	
	/*level.blockGrid = Array2d<BlockType>(50, 50);
	std::ranges::fill(level.blockGrid.span(), BlockType::EMPTY);*/
}

Editor::~Editor() {
	saveSettings();
}

void Editor::update(f32 dt) {
	rooms.update();
	movingBlocks.update();

	static bool firstFrame = true;
	if (firstFrame) {
		// modals can't be opened in the constructor, becuse it doesn't get called in between frames.
		if (lastLoadedLevel.has_value()) {
			editorTryLoadLevelFromFile(*lastLoadedLevel);
		}
		firstFrame = false;
	}

	Vec2 movement(0.0f);
	if (Input::isKeyHeld(KeyCode::A)) {
		movement.x -= 1.0f;
	}
	if (Input::isKeyHeld(KeyCode::D)) {
		movement.x += 1.0f;
	}
	if (Input::isKeyHeld(KeyCode::S)) {
		movement.y -= 1.0f;
	}
	if (Input::isKeyHeld(KeyCode::W)) {
		movement.y += 1.0f;
	}
	movement = movement.normalized();

	f32 speed = 400.0f;
	camera.pos += movement * dt * speed;

	{
		auto cursorPosWorldSpace = ::cursorPosWorldSpace(camera);
		if (Input::isMouseButtonDown(MouseButton::MIDDLE)) {
			moveGrabStartWorldPos = cursorPosWorldSpace;
		}

		if (Input::isMouseButtonUp(MouseButton::MIDDLE)) {
			moveGrabStartWorldPos = std::nullopt;
		}

		if (moveGrabStartWorldPos.has_value()) {
			camera.pos -= cursorPosWorldSpace - *moveGrabStartWorldPos;
		}
	}

	zoomOnCursorPos(camera, dt);

	if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
		const auto cursorPos = cursorPosWorldSpace(camera);

		for (const auto& room : rooms) {
			if (room.id == selectedRoomId) {
				continue;
			}

			if (roomAabb(room->position, room->blockGrid.size()).contains(cursorPos)) {
				selectedRoomId = room.id;
				break;
			}
		}
	}

	updateSelectedRoom();

	bool openOpenLevelModal = false;
	bool openCreateNewLevelModal = false;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("level")) {
			if (ImGui::MenuItem("new")) {
				openCreateNewLevelModal = true;
			} 
			if (ImGui::MenuItem("open")) {
				openOpenLevelModal = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (openCreateNewLevelModal) {
		createNewLevelInputState.openModal();
	}
	if (openOpenLevelModal) {
		openLevelModalState.openModal();
	}

	ImGui::Begin("level editor");

	if (ImGui::Button("add room")) {
		rooms.create(EditorRoom{
			.position = worldPositionToWorldGridPosition(camera.pos),
			.blockGrid = Array2d<BlockType>(45, 25),
		});
	}

	ImGui::SeparatorText("selected room");
	if (selectedRoomId.has_value()) {
		auto selectedRoom = rooms.get(*selectedRoomId);
		if (selectedRoom.has_value()) {
			if (ImGui::Button("delete")) {
				rooms.destroy(*selectedRoomId);
			}
			roomSizeGui(*selectedRoom);
		}
	} else {
		ImGui::TextWrapped("right click on a room to select it");
	}

	const std::pair<PlacableItem, const char*> placableItems[]{
		{ PlacableItem::NORMAL_BLOCK, "block" },
		{ PlacableItem::SPAWN_POINT, "spawn point" },
		{ PlacableItem::SPIKE_BOTTOM, "spike bottom" },
		{ PlacableItem::SPIKE_TOP, "spike top" },
		{ PlacableItem::SPIKE_LEFT, "spike left" },
		{ PlacableItem::SPIKE_RIGHT, "spike right" },
		{ PlacableItem::PLATFORM, "platform" },
		{ PlacableItem::DOUBLE_JUMP_ORB, "double jump orb" },
		{ PlacableItem::ATTRACTING_ORB, "attracting orb" },
		{ PlacableItem::MOVING_BLOCK, "moving block" },
		{ PlacableItem::SPIKE_TOP_RIGHT_OPEN, "spike top right open" },
		{ PlacableItem::SPIKE_TOP_LEFT_OPEN, "spike top left open" },
		{ PlacableItem::SPIKE_BOTTOM_RIGHT_OPEN, "spike bottom right open" },
		{ PlacableItem::SPIKE_BOTTOM_LEFT_OPEN, "spike bottom left open" },
		{ PlacableItem::SPIKE_TOP_RIGHT_CLOSED, "spike top right closed" },
		{ PlacableItem::SPIKE_TOP_LEFT_CLOSED, "spike top left closed" },
		{ PlacableItem::SPIKE_BOTTOM_RIGHT_CLOSED, "spike bottom right closed" },
		{ PlacableItem::SPIKE_BOTTOM_LEFT_CLOSED, "spike bottom left closed" },
	};

	ImGui::SeparatorText("block selection");
	for (const auto item : placableItems) {
		const auto type = item.first;
		const auto name = item.second;

		if (ImGui::Selectable(name, type == selectedPlacableItem)) {
			selectedPlacableItem = type;
		}
	}
	ImGui::SeparatorText("selected");
	if (selectedPlacableItem == PlacableItem::MOVING_BLOCK) {
		if (selectedMovingBlock.has_value()) {
			auto movingBlock = movingBlocks.get(*selectedMovingBlock);
			if (movingBlock.has_value()) {
				ImGui::InputFloat("speed block per second", &movingBlock->speedBlockPerSecond);
				if (movingBlock->speedBlockPerSecond < 0.0f) {
					movingBlock->speedBlockPerSecond = 0.0f;
				}
				ImGui::Checkbox("activate on collision", &movingBlock->activateOnCollision);
				ImGui::Checkbox("stop at end", &movingBlock->stopAtEnd);
			} else {
				selectedMovingBlock = std::nullopt;
			}

		} else {
			ImGui::Text("no moving block selected");
		}
	}

	ImGui::End();

	errorModal();
	{
		auto result = createNewLevelInputState.modal();
		if (result.has_value()) {
			loadNewLevel(std::move(result->level), std::move(result->name));
		}
	}
	{
		auto result = openLevelModalState.modal();
		if (result.has_value()) {
			loadNewLevel(std::move(result->level), std::move(result->name));
		}
	}
}

void Editor::updateSelectedRoom() {
	if (!selectedRoomId.has_value()) {
		return;
	}

	auto selectedEditorRoom = rooms.get(*selectedRoomId);

	if (!selectedEditorRoom.has_value()) {
		return;
	}

	auto& selectedRoom = *selectedEditorRoom;

	if (Input::isKeyDownWithAutoRepeat(KeyCode::RIGHT)) {
		std::cout << "test";
		selectedRoom.position.x += 1;
	}
	if (Input::isKeyDownWithAutoRepeat(KeyCode::LEFT)) {
		selectedRoom.position.x -= 1;
	}
	if (Input::isKeyDownWithAutoRepeat(KeyCode::UP)) {
		selectedRoom.position.y += 1;
	}
	if (Input::isKeyDownWithAutoRepeat(KeyCode::DOWN)) {
		selectedRoom.position.y -= 1;
	}

	const auto cursorPos = cursorPosWorldSpace(camera);
	const auto globalCursorGridPos = worldPositionToWorldGridPosition(cursorPos);
	const auto cursorGridPos = globalCursorGridPos - selectedRoom.position;
	const auto alignedCursorPos = Vec2(cursorGridPos) * constants().cellSize;
	const auto roomCursorPos = cursorPos - Vec2(selectedRoom.position) * constants().cellSize;

	const auto inBounds =
		cursorGridPos.x >= 0 &&
		cursorGridPos.y >= 0 &&
		cursorGridPos.x < selectedRoom.blockGrid.size().x &&
		cursorGridPos.y < selectedRoom.blockGrid.size().y;

	if (inBounds && Input::isMouseButtonHeld(MouseButton::LEFT)) {
		if (selectedPlacableItem == PlacableItem::NORMAL_BLOCK) {
			selectedRoom.blockGrid(cursorGridPos.x, cursorGridPos.y) = BlockType::NORMAL;
		}
	}

	auto placableItemToBlockType = [](PlacableItem item) -> std::optional<BlockType> {
		using enum PlacableItem;
		switch (item) {
		case NORMAL_BLOCK: return BlockType::NORMAL;
		case SPIKE_TOP:	return BlockType::SPIKE_TOP;
		case SPIKE_BOTTOM: return BlockType::SPIKE_BOTTOM;
		case SPIKE_LEFT: return BlockType::SPIKE_LEFT;
		case SPIKE_RIGHT: return BlockType::SPIKE_RIGHT;
		case PLATFORM: return BlockType::PLATFORM;
		case SPIKE_TOP_RIGHT_OPEN: return BlockType::SPIKE_TOP_RIGHT_OPEN;
		case SPIKE_TOP_LEFT_OPEN: return BlockType::SPIKE_TOP_LEFT_OPEN;
		case SPIKE_BOTTOM_RIGHT_OPEN: return BlockType::SPIKE_BOTTOM_RIGHT_OPEN;
		case SPIKE_BOTTOM_LEFT_OPEN: return BlockType::SPIKE_BOTTOM_LEFT_OPEN;
		case SPIKE_TOP_RIGHT_CLOSED: return BlockType::SPIKE_TOP_RIGHT_CLOSED;
		case SPIKE_TOP_LEFT_CLOSED: return BlockType::SPIKE_TOP_LEFT_CLOSED;
		case SPIKE_BOTTOM_RIGHT_CLOSED: return BlockType::SPIKE_BOTTOM_RIGHT_CLOSED;
		case SPIKE_BOTTOM_LEFT_CLOSED: return BlockType::SPIKE_BOTTOM_LEFT_CLOSED;

		case SPAWN_POINT:
			break;
		}
		return std::nullopt;
		};

	const auto selectedPlacableItemBlockType = placableItemToBlockType(selectedPlacableItem);

	if (selectedPlacableItemBlockType.has_value() && inBounds) {
		if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
			selectedRoom.blockGrid(cursorGridPos.x, cursorGridPos.y) = *selectedPlacableItemBlockType;
		}
		if (Input::isMouseButtonHeld(MouseButton::RIGHT)) {
			selectedRoom.blockGrid(cursorGridPos.x, cursorGridPos.y) = BlockType::EMPTY;
		}

	} else if (selectedPlacableItem == PlacableItem::SPAWN_POINT) {
		if (Input::isMouseButtonDown(MouseButton::LEFT)) {
			selectedRoom.spawnPoints.push_back(LevelSpawnPoint{
				.position = alignedCursorPos
			});
		}
		if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
			removeIf(selectedRoom.spawnPoints, [&](const LevelSpawnPoint& spawnPoint) -> bool {
				const auto position = spawnPointToPlayerSpawnPos(spawnPoint, selectedRoom.position);
				return (playerAabb(position).contains(cursorPos));
			});
		}
	} else if (selectedPlacableItem == PlacableItem::DOUBLE_JUMP_ORB) {
		if (Input::isMouseButtonDown(MouseButton::LEFT)) {
			selectedRoom.doubleJumpOrbs.push_back(LevelDoubleJumpOrb{
				.position = alignedCursorPos + Vec2(constants().cellSize / 2.0f)
			});
		}
		if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
			removeIf(selectedRoom.doubleJumpOrbs, [&](const LevelDoubleJumpOrb& orb) -> bool {
				return circleContains(
					orb.position, 
					constants().doubleJumpOrbRadius, 
					roomCursorPos);
			});
		}
	} else if (selectedPlacableItem == PlacableItem::ATTRACTING_ORB) {
		if (Input::isMouseButtonDown(MouseButton::LEFT)) {
			selectedRoom.attractingOrbs.push_back(LevelAttractingOrb{
				.position = alignedCursorPos + Vec2(constants().cellSize / 2.0f)
			});
		}
		if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
			removeIf(selectedRoom.attractingOrbs, [&](const LevelAttractingOrb& orb) -> bool {
				return circleContains(
					orb.position,
					constants().attractingOrbRadius,
					roomCursorPos);
			});
		}
	} else if (selectedPlacableItem == PlacableItem::MOVING_BLOCK) {
		struct HoveredOver {
			MovingBlockId id;
			i32 index;
		};
		std::optional<HoveredOver> hoveredOver;
		for (i32 i = 0; i < selectedRoom.movingBlocks.size(); i++) {
			const auto& blockId = selectedRoom.movingBlocks[i];
			const auto& block = movingBlocks.get(blockId);
			const auto aabb = movingBlockAabbs(*block);
			if (aabb.start.contains(roomCursorPos) || aabb.end.contains(roomCursorPos)) {
				hoveredOver = HoveredOver{
					.id = blockId,
					.index = i
				};
			}
		}
		
		if (Input::isMouseButtonDown(MouseButton::LEFT)) {
			if (hoveredOver.has_value()) {
				selectedMovingBlock = hoveredOver->id;
			} else {
				auto result = movingBlockPlaceState.onLeftClick(cursorPos, selectedEditorRoom->position);
				if (result.has_value()) {
					const auto id = movingBlocks.create(std::move(*result)).id;
					selectedEditorRoom->movingBlocks.push_back(id);
				}
			}
		}
		if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
			if (selectedMovingBlock.has_value()) {
				selectedMovingBlock = std::nullopt;
			} else {
				if (hoveredOver.has_value()) {
					movingBlocks.destroy(hoveredOver->id);
					selectedRoom.movingBlocks.erase(selectedRoom.movingBlocks.begin() + hoveredOver->index);
				}
				movingBlockPlaceState.onRightClick();
			}
		}
	}
}

void Editor::render(GameRenderer& renderer) {
	glClear(GL_COLOR_BUFFER_BIT);

	renderer.renderer.camera = camera;
	renderer.renderGrid(constants().cellSize);

	for (const auto& room : rooms) {
		renderRoom(room.entity, renderer);
	}

	if (selectedPlacableItem == PlacableItem::MOVING_BLOCK) {
		movingBlockPlaceState.render(cursorPosWorldSpace(camera));
	}

	renderer.update();
}

void Editor::renderRoom(const EditorRoom& room, GameRenderer& renderer) {
	Dbg::drawAabb(roomAabb(room.position, room.blockGrid.size()), Color3::WHITE, 2.0f);

	for (const auto& spawnPoint : room.spawnPoints) {
		const auto position = spawnPointToPlayerSpawnPos(spawnPoint, room.position);
		const auto aabb = playerAabb(position);
		Dbg::drawAabb(aabb, Color3::WHITE, 2.0f);
	}

	for (const auto& orb : room.doubleJumpOrbs) {
		renderer.renderDoubleJumpOrb(orb.position + Vec2(room.position) * constants().cellSize);
	}

	for (const auto& orb : room.attractingOrbs) {
		renderer.renderAttractingOrb(orb.position + Vec2(room.position) * constants().cellSize);
	}

	const auto roomOffset = Vec2(room.position) * constants().cellSize;

	for (const auto& blockId : room.movingBlocks) {
		const auto block = movingBlocks.get(blockId);
		if (!block.has_value()) {
			ASSERT_NOT_REACHED();
			continue;
		}
		const auto aabb = movingBlockAabbs(*block);
		Dbg::drawAabb(aabb.start.translated(roomOffset), Color3::GREEN / 2.0f, 2.0f);
		Dbg::drawAabb(aabb.end.translated(roomOffset), Color3::GREEN / 6.0f, 2.0f);
		Dbg::drawLine(aabb.start.center() + roomOffset, aabb.end.center() + roomOffset, Color3::WHITE / 2.0f, 2.0f);
	}

	for (i32 roomYi = 0; roomYi < room.blockGrid.size().y; roomYi++) {
		for (i32 roomXi = 0; roomXi < room.blockGrid.size().x; roomXi++) {
			const auto cellBottomLeft = Vec2(roomXi, roomXi) * constants().cellSize;
			const auto cellTopRight = Vec2(roomXi + 1, roomXi + 1) * constants().cellSize;

			const auto xi = roomXi + room.position.x;
			const auto yi = roomYi + room.position.y;

			switch (room.blockGrid(roomXi, roomYi)) {
				using enum BlockType;
			case NORMAL: {
				const auto directions = getBlockCollisionDirections(room.blockGrid, roomXi, roomYi);
				const Block block{
					.position = Vec2(xi, yi) * constants().cellSize,
					.collisionDirections = directions,
				};
				renderer.renderBlock(block);
				break;
			}

			case SPIKE_LEFT:
				renderer.renderSpike(makeSpikeLeft(roomXi, roomYi, room.position));
				break;
			case SPIKE_RIGHT:
				renderer.renderSpike(makeSpikeRight(roomXi, roomYi, room.position));
				break;
			case SPIKE_TOP:
				renderer.renderSpike(makeSpikeTop(roomXi, roomYi, room.position));
				break;
			case SPIKE_BOTTOM:
				renderer.renderSpike(makeSpikeBottom(roomXi, roomYi, room.position));
				break;

			case PLATFORM:
				renderer.renderPlatform(makePlatform(roomXi, roomYi, room.position));
				break;

			case EMPTY:
				break;

			}
		}
	}
}

void Editor::roomSizeGui(EditorRoom& room) {
	Gui::put("% x %", room.blockGrid.size().x, room.blockGrid.size().y);
	auto increaseLeftAndBottom = [](const Array2d<BlockType>& array, i32 increaseX, i32 increaseY) -> Array2d<BlockType> {
		auto n = Array2d<BlockType>::withAllSetTo(
			array.size().x + increaseX,
			array.size().y + increaseY,
			BlockType::EMPTY);

		n.paste(increaseX, increaseY, array);
		return n;
	};

	auto decreaseLeftAndBottom = [](const Array2d<BlockType>& array, i32 decreaseX, i32 decreaseY) -> Array2d<BlockType> {
		auto n = Array2d<BlockType>::withAllSetTo(
			std::max(array.size().x - decreaseX, 0ll),
			std::max(array.size().y - decreaseY, 0ll),
			BlockType::EMPTY);

		n.paste(0, 0, array, decreaseX, decreaseY);
		return n;
	};

	auto increaseRightAndTop = [](const Array2d<BlockType>& array, i32 increaseX, i32 increaseY) -> Array2d<BlockType> {
		auto n = Array2d<BlockType>::withAllSetTo(
			array.size().x + increaseX,
			array.size().y + increaseY,
			BlockType::EMPTY);

		n.paste(0, 0, array);
		return n;
	};

	auto decreaseRightAndTop = [](const Array2d<BlockType>& array, i32 decreaseX, i32 decreaseY) -> Array2d<BlockType> {
		auto n = Array2d<BlockType>::withAllSetTo(
			std::max(array.size().x - decreaseX, 0ll),
			std::max(array.size().y - decreaseY, 0ll),
			BlockType::EMPTY);

		n.paste(0, 0, array);
		return n;
	};

	ImGui::InputInt("increase or decrease size", &increaseOrDecreaseSize);
	increaseOrDecreaseSize = std::max(1, increaseOrDecreaseSize);
	{
		ImGui::PushID("left");
		if (ImGui::Button("+")) {
			room.blockGrid = increaseLeftAndBottom(room.blockGrid, increaseOrDecreaseSize, 0);
			room.position.x -= increaseOrDecreaseSize;
			moveObjects(room, Vec2T<i32>(1, 0));
		}
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			room.blockGrid = decreaseLeftAndBottom(room.blockGrid, increaseOrDecreaseSize, 0);
			room.position.x += increaseOrDecreaseSize;
			moveObjects(room, Vec2T<i32>(-1, 0));
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::Text("left");
	}
	

	{
		ImGui::PushID("right");
		if (ImGui::Button("+")) {
			room.blockGrid = increaseRightAndTop(room.blockGrid, increaseOrDecreaseSize, 0);
		}
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			room.blockGrid = decreaseRightAndTop(room.blockGrid, increaseOrDecreaseSize, 0);
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::Text("right");
	}

	{
		ImGui::PushID("bottom");
		if (ImGui::Button("+")) {
			room.blockGrid = increaseLeftAndBottom(room.blockGrid, 0, increaseOrDecreaseSize);
			moveObjects(room, Vec2T<i32>(0, 1));
			room.position.y -= increaseOrDecreaseSize;
		}
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			room.blockGrid = decreaseLeftAndBottom(room.blockGrid, 0, increaseOrDecreaseSize);
			moveObjects(room, Vec2T<i32>(0, -1));
			room.position.y += increaseOrDecreaseSize;
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::Text("bottom");
	}

	{
		ImGui::PushID("top");
		if (ImGui::Button("+")) {
			room.blockGrid = increaseRightAndTop(room.blockGrid, 0, increaseOrDecreaseSize);
		}
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			room.blockGrid = decreaseRightAndTop(room.blockGrid, 0, increaseOrDecreaseSize);
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::Text("top");
	}
}

void Editor::moveObjects(EditorRoom& room, Vec2T<i32> change) {
	auto updatePosition = [&](Vec2 p) -> Vec2 {
		return p + Vec2(change) * constants().cellSize;
	};

	for (auto& spawnPoint : room.spawnPoints) {
		spawnPoint.position = updatePosition(spawnPoint.position);
	}
	for (auto& orb : room.doubleJumpOrbs) {
		orb.position = updatePosition(orb.position);
	}
	for (auto& orb : room.attractingOrbs) {
		orb.position = updatePosition(orb.position);
	}
	for (auto& blockId : room.movingBlocks) {
		auto block = movingBlocks.get(blockId);
		if (!block.has_value()) {
			ASSERT_NOT_REACHED();
			continue;
		}
		block->position = updatePosition(block->position);
		block->endPosition = updatePosition(block->endPosition);
	}
}

Vec2T<i32> Editor::worldPositionToWorldGridPosition(Vec2 worldPosition) {
	return Vec2T<i32>((worldPosition / constants().cellSize).applied(floor));
}

Vec2 Editor::worldPositionRoundedToGrid(Vec2 worldPosition) {
	const auto gridPosition = worldPositionToWorldGridPosition(worldPosition);
	return Vec2(gridPosition) * constants().cellSize;
}

Editor::OnSwitchToGameResult Editor::onSwitchToGame() {
	if (!lastLoadedLevel.has_value()) {
		return OnSwitchToGameResult{ .selectedRoomIndex = std::nullopt };
	}
	auto result = generateLevel2();
	saveLevelToFile(*lastLoadedLevel, std::move(result.level));
	return OnSwitchToGameResult{
		.selectedRoomIndex = result.selectedRoomIndex
	};
}

void Editor::onSwitchFromGame() {
	if (!lastLoadedLevel.has_value()) {
		return;
	}

	tryLoadLevelFromFile(*lastLoadedLevel);
}

void Editor::loadNewLevel(Level&& level, std::string&& newLevelName) {
	if (lastLoadedLevel.has_value()) {
		saveLevelToFile(*lastLoadedLevel, generateLevel());
	}
	loadLevel(std::move(level));
	lastLoadedLevel = newLevelName;
	saveSettings();
}

void Editor::editorTryLoadLevelFromFile(std::string_view path) {
	auto level = tryLoadLevelFromFile(path);
	if (!level.has_value()) {
		errorModalMessage.clear();
		StringRefStream msg(errorModalMessage);
		msg << "failed to load '" << path << "'";
		openErrorModal();
		return;
	}

	loadLevel(std::move(*level));
}

void Editor::saveSettings() {
	if (lastLoadedLevel.has_value()) {
		EditorSettings settings{
			.lastLoadedLevel = *lastLoadedLevel,
			.lastLoadedCameraPosition = camera.pos,
			.lastLoadedCameraZoom = camera.zoom
		};
 		const auto json = toJson(settings);
		std::ofstream file(EDITOR_SETTINGS_PATH);
		Json::prettyPrint(file, json);
	}
}

void Editor::openErrorModal() {
	ImGui::OpenPopup(errorModalName);
} 

void Editor::errorModal() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(errorModalName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	ImGui::Text("%s", errorModalMessage.c_str());

	if (ImGui::Button("ok")) {
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void Editor::loadLevel(Level&& level) {
	for (auto& room : level.rooms) {
		EditorRoom r{
			.position = room.position,
			.blockGrid = std::move(room.blockGrid),
			.spawnPoints = std::move(room.spawnPoints),
			.doubleJumpOrbs = std::move(room.doubleJumpOrbs),
			.attractingOrbs = std::move(room.attractingOrbs)
		};
		for (auto& movingBlock : room.movingBlocks) {
			const auto id = movingBlocks.create(std::move(movingBlock)).id;
			r.movingBlocks.push_back(id);
		}
		rooms.create(std::move(r));
	}
}

Level Editor::generateLevel() {
	return generateLevel2().level;
}

Editor::GenerateLevel2Result Editor::generateLevel2() {
	Level level;
	std::optional<i32> selectedRoomIndex;
	for (const auto& room : rooms) {
		if (room.id == selectedRoomId) {
			selectedRoomIndex = level.rooms.size();
		}

		level.rooms.emplace_back(LevelRoom{
			.position = room->position,
			.blockGrid = Array2d<BlockType>(room->blockGrid),
			.spawnPoints = room->spawnPoints,
			.doubleJumpOrbs = room->doubleJumpOrbs,
			.attractingOrbs = room->attractingOrbs
		});
		auto& levelRoom = level.rooms.back();
		for (const auto& id : room->movingBlocks) {
			const auto block = movingBlocks.get(id);
			if (block.has_value()) {
				levelRoom.movingBlocks.push_back(*block);
			}
		}
	}
	return GenerateLevel2Result{ .level = std::move(level), .selectedRoomIndex = selectedRoomIndex};
}

const char* createNewLevelModalName = "new level";
void Editor::CreateNewLevelInputState::openModal() {
	gridSize = Vec2T<i32>(50);
	name.clear();
	ImGui::OpenPopup(createNewLevelModalName);
}

std::optional<Editor::CreateNewLevelInputState::Result> Editor::CreateNewLevelInputState::modal() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(createNewLevelModalName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return std::nullopt;
	}

	ImGui::InputText("name", &name);
	ImGui::InputInt2("gridSize", gridSize.data());

	if (ImGui::Button("create")) {
		const auto path = std::filesystem::path(levelsPath) / name;
		const auto alreadyTaken = std::filesystem::exists(path);
		if (alreadyTaken) {
			return std::nullopt;
		}
		ImGui::CloseCurrentPopup();
		Result result{
			.name = path.string(),
			/*.level = Level{ .blockGrid = Array2d<BlockType>(gridSize.x, gridSize.y) },*/
			.level = Level{ },
		};
		//std::ranges::fill(result.level.blockGrid.span(), BlockType::EMPTY);
		ImGui::EndPopup();
		return result;
	}

	ImGui::SameLine();

	if (ImGui::Button("cancel")) {
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();

	return std::nullopt;
}

const char* openLevelModalName = "open level";
void Editor::OpenLevelModalState::openModal() {
	name.clear();
	ImGui::OpenPopup(openLevelModalName);
}

std::optional<Editor::OpenLevelModalState::Result> Editor::OpenLevelModalState::modal() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(openLevelModalName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return std::nullopt;
	}

	ImGui::InputText("name", &name);

	if (ImGui::Button("open")) {
		const auto path = std::filesystem::path(levelsPath) / name;
		auto pathString = path.string();
		auto result = tryLoadLevelFromFile(pathString);
		if (result.has_value()) {
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return Result{
				.name = std::move(pathString),
				.level = std::move(*result),
			};
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("cancel")) {
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();

	return std::nullopt;
}

std::optional<LevelMovingBlock> Editor::MovingBlockPlaceState::onLeftClick(Vec2 globalCursorPos, Vec2T<i32> roomPosition) {

	const auto cursorPosRounded = Editor::worldPositionRoundedToGrid(globalCursorPos);

	if (!blockCorner0.has_value()) {
		blockCorner0 = cursorPosRounded;
		return std::nullopt;
	}

	if (!blockCorner1.has_value()) {
		blockCorner1 = cursorPosRounded + Vec2(1.0f);
		return std::nullopt;
	}

	const auto roomOffset = Vec2(roomPosition) * constants().cellSize;

	const auto blockAabb = Aabb::fromCorners(*blockCorner0, *blockCorner1);

	LevelMovingBlock result{
		.position = blockAabb.min - roomOffset,
		.size = blockAabb.size(),
		.endPosition = cursorPosRounded - roomOffset
	};

	blockCorner0 = std::nullopt;
	blockCorner1 = std::nullopt;

	return result;
}

void Editor::MovingBlockPlaceState::onRightClick() {
	blockCorner0 = std::nullopt;
	blockCorner1 = std::nullopt;
}

void Editor::MovingBlockPlaceState::render(Vec2 globalCursorPos) {
	const auto cursor = worldPositionRoundedToGrid(globalCursorPos);

	if (blockCorner0.has_value() && !blockCorner1.has_value()) {
		Dbg::drawAabb(Aabb::fromCorners(*blockCorner0, cursor + Vec2(1.0f)), Color3::WHITE / 2.0f, 2.0f);
	}

	if (blockCorner0.has_value() && blockCorner1.has_value()) {
		const auto aabb = Aabb::fromCorners(*blockCorner0, *blockCorner1);
		Dbg::drawAabb(aabb, Color3::GREEN, 2.0f);
		const auto preview = Aabb(cursor, cursor + aabb.size());
		Dbg::drawAabb(preview, Color3::WHITE / 2.0f, 2.0f);
		Dbg::drawLine(aabb.center(), preview.center(), Color3::WHITE / 2.0f, 2.0f);
	}

	
}
