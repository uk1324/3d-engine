#include <water_simulation/Camera3d.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>

#include <imgui/imgui.h>

void Camera3d::update(float dt) {
	Vec2 mouseOffset;
	if (lastMousePosition.has_value()) {
		mouseOffset = Input::cursorPosWindowSpace() - *lastMousePosition;
	} else {
		mouseOffset = Vec2(0.0f);
	}

	lastMousePosition = Input::cursorPosWindowSpace();

	float horizontalChange = mouseOffset.x * rotationSpeed * dt;
	float verticalChange = -mouseOffset.y * rotationSpeed * dt;
	const auto right = cross(forward, up);
	const auto rotation = Quat(verticalChange, right) * Quat(horizontalChange, up);
	forward *= rotation;

	Vec3 movementDirection(0.0f);

	if (Input::isKeyHeld(KeyCode::A)) movementDirection += Vec3::LEFT;
	if (Input::isKeyHeld(KeyCode::D)) movementDirection += Vec3::RIGHT;
	if (Input::isKeyHeld(KeyCode::W)) movementDirection += Vec3::FORWARD;
	if (Input::isKeyHeld(KeyCode::S)) movementDirection += Vec3::BACK;
	if (Input::isKeyHeld(KeyCode::SPACE)) movementDirection += Vec3::UP;
	if (Input::isKeyHeld(KeyCode::LEFT_SHIFT)) movementDirection += Vec3::DOWN;

	float roll = 0.0f;
	if (Input::isKeyHeld(KeyCode::LEFT)) {
		roll += dt;
	}
	if (Input::isKeyHeld(KeyCode::RIGHT)) {
		roll -= dt;
	}
	up *= Quat(roll, forward);

	movementDirection = movementDirection.normalized();

	//Quat rotationAroundYAxis(angleAroundUpAxis, Vec3::UP);
	const auto dir = Vec4(movementDirection, 0.0f) * viewMatrix().inversed() * movementSpeed * dt;
	position += dir.xyz();

	const auto angle = acos(dot(up, forward));
	auto axis = cross(up, forward);
	const auto maxAllowedDifference = 0.05f;
	ImGui::Text("%g", angle);
	ImGui::InputFloat3("g", forward.data());
	ImGui::InputFloat3("a", axis.data());
	if (std::abs(angle) < maxAllowedDifference) {
		forward = up * Quat(std::signbit(angle) * maxAllowedDifference, axis.normalized());
	}

	forward = forward.normalized();
	up = up.normalized();
}

Mat4 Camera3d::viewMatrix() const {
	auto target = position + forward;
	return Mat4::lookAt(position, target, up);
}
