#include <platformer/Editor.hpp>
#include <framework/Dbg.hpp>
#include <framework/CameraUtils.hpp>
#include <engine/Math/Color.hpp>
#include <imgui/imgui_stdlib.h>
#include <engine/Input/Input.hpp>
#include <StringRefStream.hpp>
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <filesystem>

Editor::Editor() {
	camera.zoom /= 500.0f;
	/*level.blockGrid = Array2d<BlockType>(50, 50);
	std::ranges::fill(level.blockGrid.span(), BlockType::EMPTY);*/
}

void Editor::update(f32 dt, f32 cellSize) {

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

	{
		const auto cursorPos = cursorPosWorldSpace(camera);
		const auto cursorGridPos = Vec2T<i32>((cursorPos / cellSize).applied(floor));

		const auto inBounds =
			cursorGridPos.x >= 0 &&
			cursorGridPos.y >= 0 &&
			cursorGridPos.x < level.blockGrid.size().x &&
			cursorGridPos.y < level.blockGrid.size().y;

		if (inBounds && Input::isMouseButtonHeld(MouseButton::LEFT)) {
			if (selectedPlacableItem == PlacableItem::NORMAL_BLOCK) {
				level.blockGrid(cursorGridPos.x, cursorGridPos.y) = BlockType::NORMAL;
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

			case LEVEL_TRANSITON:
				break;
			}
			return std::nullopt;
		};

		const auto selectedPlacableItemBlockType = placableItemToBlockType(selectedPlacableItem);

		if (selectedPlacableItemBlockType.has_value() && inBounds) {
			if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
				level.blockGrid(cursorGridPos.x, cursorGridPos.y) = *selectedPlacableItemBlockType;
			}
			if (Input::isMouseButtonHeld(MouseButton::RIGHT)) {
				level.blockGrid(cursorGridPos.x, cursorGridPos.y) = BlockType::EMPTY;
			}

		} else if (selectedPlacableItem == PlacableItem::LEVEL_TRANSITON) {
			if (Input::isMouseButtonDown(MouseButton::LEFT)) {
				const auto result = lavelTransitionPlaceState.onLeftClick(Vec2(cursorGridPos) * cellSize, cellSize);
				if (result.has_value()) {
					level.levelTransitions.push_back(*result);
				}
			}
			if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
				lavelTransitionPlaceState.onRightClick();
			}
		}

		/*if (inBounds && Input::isMouseButtonDown(MouseButton::LEFT) ) {
			
				level.levelTransitions.push_back(LevelTransition{ 
					.respawnPoint = Vec2(cursorGridPos) * cellSize 
				});
			}
		}*/

		

	}

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

	if (ImGui::Button("normal block")) {
		selectedPlacableItem = PlacableItem::NORMAL_BLOCK;
	} else if (ImGui::Button("level transition")) {
		selectedPlacableItem = PlacableItem::LEVEL_TRANSITON;
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

	if (ImGui::CollapsingHeader("level transitions")) {
		if (lastLoadedLevel.has_value()) {
			ImGui::PushID(lastLoadedLevel->c_str());
		}
		const auto [f, l] = std::ranges::remove_if(
			level.levelTransitions,
			[&](LevelTransition& transition) -> bool {
				Vec2 idData[2] = { transition.trigger.min, transition.trigger.max };
				const auto id = reinterpret_cast<const char*>(idData);
				ImGui::PushID(id, id + sizeof(idData));
				ImGui::InputText("level", &transition.level);

				bool remove = false;
				if (ImGui::Button("delete")) {
					remove = true;
				}

				ImGui::PopID();
				return remove;

			}
		);
		level.levelTransitions.erase(f, l);
		if (lastLoadedLevel.has_value()) {
			ImGui::PopID();
		}
		//std::e
		//if (ImGui::Button("add")) {
		//	level.levelTransitions.push_back(LevelTransition)
		//}
		//for (i32 i = 0; i < level.levelTransitions.size(); i++) {
		//	//ImGui::InputFloat("`")
		//}

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

void Editor::render(GameRenderer& renderer, f32 cellSize, const PlayerSettings& playerSettings) {
	glClear(GL_COLOR_BUFFER_BIT);

	renderer.renderer.camera = camera;
	renderer.renderGrid(cellSize);

	Dbg::drawAabb(Vec2(0.0f), Vec2(level.blockGrid.size()) * cellSize, Color3::WHITE, 2.0f);

	for (i32 yi = 0; yi < level.blockGrid.size().y; yi++) {
		for (i32 xi = 0; xi < level.blockGrid.size().x; xi++) {
			const auto cellBottomLeft = Vec2(xi, yi) * cellSize;
			const auto cellTopRight = Vec2(xi + 1, yi + 1) * cellSize;

			switch (level.blockGrid(xi, yi)) {
				using enum BlockType;
			case NORMAL: {
				const auto directions = getBlockCollisionDirections(level.blockGrid, xi, yi);
				const Block block{
					.position = Vec2(xi, yi) * cellSize,
					.collisionDirections = directions,
				};
				renderer.renderBlock(block, cellSize);
				break;
			}

			case SPIKE_LEFT:
				renderer.renderSpike(makeSpikeLeft(xi, yi, cellSize));
				break;
			case SPIKE_RIGHT:
				renderer.renderSpike(makeSpikeRight(xi, yi, cellSize));
				break;
			case SPIKE_TOP:
				renderer.renderSpike(makeSpikeTop(xi, yi, cellSize));
				break;
			case SPIKE_BOTTOM:
				renderer.renderSpike(makeSpikeBottom(xi, yi, cellSize));
				break;
				
			case PLATFORM:
				renderer.renderPlatform(makePlatform(xi, yi, cellSize), cellSize);
				break;

			case EMPTY:
				break;

			}
		}
	}

	for (const auto& transition : level.levelTransitions) {
		const auto position = levelTransitionToPlayerSpawnPos(transition, playerSettings);

		Dbg::drawAabb(
			position - playerSettings.size / 2.0f,
			position + playerSettings.size / 2.0f,
			Color3::WHITE,
			2.0f);

		Dbg::drawAabb(transition.trigger.min, transition.trigger.max, Color3::BLUE, 2.0f);
	}

	renderer.update();
}

void Editor::onSwitchToGame() {
	if (!lastLoadedLevel.has_value()) {
		return;
	}
	saveLevelToFile(*lastLoadedLevel, level);
}

void Editor::onSwitchFromGame() {
	if (!lastLoadedLevel.has_value()) {
		return;
	}

	auto level = tryLoadLevelFromFile(*lastLoadedLevel);
	if (!level.has_value()) {
		errorModalMessage.clear();
		StringRefStream msg(errorModalMessage);
		msg << "failed to load '" << *lastLoadedLevel << "'";
		openErrorModal();
		return;
	}

	this->level = std::move(*level);
}

void Editor::loadNewLevel(Level&& level, std::string&& newLevelName) {
	if (lastLoadedLevel.has_value()) {
		saveLevelToFile(*lastLoadedLevel, this->level);
	}
	this->level = std::move(level);
	lastLoadedLevel = newLevelName;
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

std::optional<LevelTransition> Editor::LevelTransitionPlaceState::onLeftClick(Vec2 cursorPos, f32 cellSize) {
	if (!triggerCorner0.has_value()) {
		triggerCorner0 = cursorPos;
		return std::nullopt;
	}

	if (!triggerCorner1.has_value()) {
		triggerCorner1 = cursorPos;
		return std::nullopt;
	}

	Aabb triggerHitbox = Aabb::fromCorners(*triggerCorner0, *triggerCorner1);
	triggerHitbox.max += Vec2(cellSize);

	triggerCorner0 = std::nullopt;
	triggerCorner1 = std::nullopt;

	return LevelTransition{
		.respawnPoint = cursorPos,
		.trigger = triggerHitbox,
	};
}

void Editor::LevelTransitionPlaceState::onRightClick() {
	triggerCorner0 = std::nullopt;
	triggerCorner1 = std::nullopt;
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
			.level = Level{ .blockGrid = Array2d<BlockType>(gridSize.x, gridSize.y) },
		};
		std::ranges::fill(result.level.blockGrid.span(), BlockType::EMPTY);
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
