#include <platformer/Editor.hpp>
#include <engine/Input/Input.hpp>
#include <glad/glad.h>
#include <imgui/imgui.h>

Editor::Editor() {
	camera.zoom /= 500.0f;
}

void Editor::update(f32 dt) {

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

	auto cursorPosWorldSpace = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
	ImGui::InputFloat2("test", cursorPosWorldSpace.data());
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

void Editor::render(GameRenderer& renderer, f32 cellSize) {
	glClear(GL_COLOR_BUFFER_BIT);
	//camera.zoom = 0.001f;

	renderer.renderer.camera = camera;
	renderer.renderGrid(cellSize);
}
