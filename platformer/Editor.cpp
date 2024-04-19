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
#include <FileIo.hpp>
#include <filesystem>
#include <platformer/EditorSettingsData.hpp>
#include <engine/Json/JsonParser.hpp>
#include <engine/Json/JsonPrinter.hpp>

static constexpr auto EDITOR_SETTINGS_PATH = "generated/editorSettings.json";

Editor::Editor() {
	camera.zoom /= 500.0f;

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
	}

	
	
	/*level.blockGrid = Array2d<BlockType>(50, 50);
	std::ranges::fill(level.blockGrid.span(), BlockType::EMPTY);*/
}

Editor::~Editor() {
	saveSettings();
}

void Editor::update(f32 dt, f32 cellSize, const PlayerSettings& playerSettings) {
	rooms.update();

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

			if (roomAabb(room->levelRoom, cellSize).contains(cursorPos)) {
				selectedRoomId = room.id;
				break;
			}
		}
	}

	updateSelectedRoom(cellSize, playerSettings);

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
			.levelRoom = LevelRoom{
				.position = Vec2T<i32>(0, 0),
				.blockGrid = Array2d<BlockType>(45, 25)
			}
		});
	}

	ImGui::SeparatorText("selected room");
	if (selectedRoomId.has_value()) {
		auto selectedRoom = rooms.get(*selectedRoomId);
		if (selectedRoom.has_value()) {
			if (ImGui::Button("delete")) {
				rooms.destroy(*selectedRoomId);
			}
			roomSizeGui(selectedRoom->levelRoom, cellSize);
		}
	} else {
		ImGui::Text("no room selected");
	}

	if (ImGui::Button("normal block")) {
		selectedPlacableItem = PlacableItem::NORMAL_BLOCK;
	} else if (ImGui::Button("spawn point")) {
		selectedPlacableItem = PlacableItem::SPAWN_POINT;
	} else if (ImGui::Button("spike bottom")) {
		selectedPlacableItem = PlacableItem::SPIKE_BOTTOM;
	} else if (ImGui::Button("spike top")) {
		selectedPlacableItem = PlacableItem::SPIKE_TOP;
	} else if (ImGui::Button("spike left")) {
		selectedPlacableItem = PlacableItem::SPIKE_LEFT;
	} else if (ImGui::Button("spike right")) {
		selectedPlacableItem = PlacableItem::SPIKE_RIGHT;
	} else if (ImGui::Button("platform")) {
		selectedPlacableItem = PlacableItem::PLATFORM;
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

void Editor::updateSelectedRoom(f32 cellSize, const PlayerSettings& playerSettings) {
	if (!selectedRoomId.has_value()) {
		return;
	}

	auto selectedEditorRoom = rooms.get(*selectedRoomId);

	if (!selectedEditorRoom.has_value()) {
		return;
	}

	auto& selectedRoom = selectedEditorRoom->levelRoom;

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
	const auto globalCursorGridPos = Vec2T<i32>((cursorPos / cellSize).applied(floor));
	const auto cursorGridPos = globalCursorGridPos - selectedRoom.position;

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
				.position = Vec2(cursorGridPos) * cellSize
			});
		}
		if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
			auto& sp = selectedRoom.spawnPoints;
			sp.erase(std::remove_if(
				sp.begin(),
				sp.end(),
				[&](const LevelSpawnPoint& spawnPoint) -> bool { 
					const auto position = spawnPointToPlayerSpawnPos(spawnPoint, playerSettings, selectedRoom.position, cellSize);
					Aabb aabb(position - playerSettings.size / 2.0f, position + playerSettings.size / 2.0f);
					if (aabb.contains(cursorPos)) {
						return true;
					}
				}
			), sp.end());
		}
	}
}

void Editor::render(GameRenderer& renderer, f32 cellSize, const PlayerSettings& playerSettings) {
	glClear(GL_COLOR_BUFFER_BIT);

	renderer.renderer.camera = camera;
	renderer.renderGrid(cellSize);

	for (const auto& room : rooms) {
		renderRoom(room->levelRoom, renderer, cellSize, playerSettings);
	}

	renderer.update();
}

void Editor::renderRoom(const LevelRoom& room, GameRenderer& renderer, f32 cellSize, const PlayerSettings& playerSettings) {
	Dbg::drawAabb(roomAabb(room, cellSize), Color3::WHITE, 2.0f);

	for (const auto& spawnPoint : room.spawnPoints) {
		const auto position = spawnPointToPlayerSpawnPos(spawnPoint, playerSettings, room.position, cellSize);
		Dbg::drawAabb(
			position - playerSettings.size / 2.0f,
			position + playerSettings.size / 2.0f,
			Color3::WHITE,
			2.0f);
	}

	for (i32 roomYi = 0; roomYi < room.blockGrid.size().y; roomYi++) {
		for (i32 roomXi = 0; roomXi < room.blockGrid.size().x; roomXi++) {
			const auto cellBottomLeft = Vec2(roomXi, roomXi) * cellSize;
			const auto cellTopRight = Vec2(roomXi + 1, roomXi + 1) * cellSize;

			const auto xi = roomXi + room.position.x;
			const auto yi = roomYi + room.position.y;

			switch (room.blockGrid(roomXi, roomYi)) {
				using enum BlockType;
			case NORMAL: {
				const auto directions = getBlockCollisionDirections(room.blockGrid, roomXi, roomYi);
				const Block block{
					.position = Vec2(xi, yi) * cellSize,
					.collisionDirections = directions,
				};
				renderer.renderBlock(block, cellSize);
				break;
			}

			case SPIKE_LEFT:
				renderer.renderSpike(makeSpikeLeft(roomXi, roomYi, cellSize, room.position));
				break;
			case SPIKE_RIGHT:
				renderer.renderSpike(makeSpikeRight(roomXi, roomYi, cellSize, room.position));
				break;
			case SPIKE_TOP:
				renderer.renderSpike(makeSpikeTop(roomXi, roomYi, cellSize, room.position));
				break;
			case SPIKE_BOTTOM:
				renderer.renderSpike(makeSpikeBottom(roomXi, roomYi, cellSize, room.position));
				break;

			case PLATFORM:
				renderer.renderPlatform(makePlatform(roomXi, roomYi, cellSize, room.position), cellSize);
				break;

			case EMPTY:
				break;

			}
		}
	}
}

void Editor::roomSizeGui(LevelRoom& room, f32 cellSize) {
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
			moveObjects(room, Vec2T<i32>(1, 0), cellSize);
		}
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			room.blockGrid = decreaseLeftAndBottom(room.blockGrid, increaseOrDecreaseSize, 0);
			room.position.x += increaseOrDecreaseSize;
			moveObjects(room, Vec2T<i32>(-1, 0), cellSize);
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
			moveObjects(room, Vec2T<i32>(0, 1), cellSize);
			room.position.y -= increaseOrDecreaseSize;
		}
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			room.blockGrid = decreaseLeftAndBottom(room.blockGrid, 0, increaseOrDecreaseSize);
			moveObjects(room, Vec2T<i32>(0, -1), cellSize);
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

	//if (ImGui::Button("increase left")) {
	//	const auto increase = 1;
	//	auto n = Array2d<BlockType>(
	//		room.blockGrid.size().x + increase,
	//		room.blockGrid.size().y);

	//	for (i32 y = 0; y < n.size().y; y++) {
	//		for (i32 x = 0; x < increase; x++) {
	//			n(x, y) = BlockType::EMPTY;
	//		}
	//	}

	//	for (i32 y = 0; y < n.size().y; y++) {
	//		for (i32 x = increase; x < n.size().x; x++) {
	//			n(x, y) = room.blockGrid(x - increase, y);
	//		}
	//	}
	//	room.blockGrid = std::move(n);
	//	room.position.x -= increase;
	//}

	//if (ImGui::Button("decrease left")) {
	//	const auto decrase = 1;
	//	auto n = Array2d<BlockType>(
	//		room.blockGrid.size().x - decrase,
	//		room.blockGrid.size().y);

	//	for (i32 y = 0; y < n.size().y; y++) {
	//		for (i32 x = 0; x < n.size().x; x++) {
	//			n(x, y) = room.blockGrid(x + decrase, y);
	//		}
	//	}
	//	room.blockGrid = std::move(n);
	//	room.position.x += decrase;
	//}

	//if (ImGui::Button("increase right")) {
	//	const auto increase = 1;
	//	auto n = Array2d<BlockType>(
	//		room.blockGrid.size().x + increase,
	//		room.blockGrid.size().y);

	//	for (i32 y = 0; y < n.size().y; y++) {
	//		for (i32 x = 0; x < increase; x++) {
	//			n(room.blockGrid.size().x + x, y) = BlockType::EMPTY;
	//		}
	//	}

	//	for (i32 y = 0; y < n.size().y; y++) {
	//		for (i32 x = increase; x < n.size().x; x++) {
	//			n(x, y) = room.blockGrid(x, y);
	//		}
	//	}
	//	room.blockGrid = std::move(n);
	//}

	//if (ImGui::Button("decrease right")) {
	//	const auto decrase = 1;
	//	auto n = Array2d<BlockType>(
	//		room.blockGrid.size().x - decrase,
	//		room.blockGrid.size().y);

	//	for (i32 y = 0; y < n.size().y; y++) {
	//		for (i32 x = 0; x < n.size().x; x++) {
	//			n(x, y) = room.blockGrid(x, y);
	//		}
	//	}
	//	room.blockGrid = std::move(n);
	//}
}

void Editor::moveObjects(LevelRoom& room, Vec2T<i32> change, f32 cellSize) {
	for (auto& spawnPoint : room.spawnPoints) {
		spawnPoint.position += Vec2(change) * cellSize;
	}
}

void Editor::onSwitchToGame() {
	if (!lastLoadedLevel.has_value()) {
		return;
	}
	saveLevelToFile(*lastLoadedLevel, generateLevel());
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
			.lastLoadedLevel = *lastLoadedLevel
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
		rooms.create(EditorRoom{
			.levelRoom = std::move(room)
		});
	}
}

Level Editor::generateLevel() {
	Level level;
	for (const auto& room : rooms) {
		level.rooms.emplace_back(LevelRoom{
			.position = room->levelRoom.position,
			.blockGrid = Array2d<BlockType>(room->levelRoom.blockGrid),
			.spawnPoints = room->levelRoom.spawnPoints,
		});
	}
	return level;
}

//std::optional<LevelTransition> Editor::LevelTransitionPlaceState::onLeftClick(Vec2 cursorPos, f32 cellSize) {
//	if (!triggerCorner0.has_value()) {
//		triggerCorner0 = cursorPos;
//		return std::nullopt;
//	}
//
//	if (!triggerCorner1.has_value()) {
//		triggerCorner1 = cursorPos;
//		return std::nullopt;
//	}
//
//	Aabb triggerHitbox = Aabb::fromCorners(*triggerCorner0, *triggerCorner1);
//	triggerHitbox.max += Vec2(cellSize);
//
//	triggerCorner0 = std::nullopt;
//	triggerCorner1 = std::nullopt;
//
//	return LevelTransition{
//		.respawnPoint = cursorPos,
//		.trigger = triggerHitbox,
//	};
//}

//void Editor::LevelTransitionPlaceState::onRightClick() {
//	triggerCorner0 = std::nullopt;
//	triggerCorner1 = std::nullopt;
//}

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
