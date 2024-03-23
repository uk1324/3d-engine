

//#include <water_simulation/Camera3d.hpp>
//#include <engine/Input/Input.hpp>
//#include <engine/Math/Angles.hpp>
//
//#include <imgui/imgui.h>
//
//void Camera3d::update(float dt) {
//	Vec2 mouseOffset;
//	if (lastMousePosition.has_value()) {
//		mouseOffset = Input::cursorPosWindowSpace() - *lastMousePosition;
//	} else {
//		mouseOffset = Vec2(0.0f);
//	}
//
//	lastMousePosition = Input::cursorPosWindowSpace();
//
//	float horizontalChange = mouseOffset.x * rotationSpeed * dt;
//	float verticalChange = -mouseOffset.y * rotationSpeed * dt;
//	const auto right = cross(forward, up);
//	const auto rotation = Quat(verticalChange, right) * Quat(horizontalChange, up);
//	forward *= rotation;
//
//	Vec3 movementDirection(0.0f);
//
//	if (Input::isKeyHeld(KeyCode::A)) movementDirection += Vec3::LEFT;
//	if (Input::isKeyHeld(KeyCode::D)) movementDirection += Vec3::RIGHT;
//	if (Input::isKeyHeld(KeyCode::W)) movementDirection += Vec3::FORWARD;
//	if (Input::isKeyHeld(KeyCode::S)) movementDirection += Vec3::BACK;
//	if (Input::isKeyHeld(KeyCode::SPACE)) movementDirection += Vec3::UP;
//	if (Input::isKeyHeld(KeyCode::LEFT_SHIFT)) movementDirection += Vec3::DOWN;
//
//	movementDirection = movementDirection.normalized();
//
//	const auto dir = Vec4(movementDirection, 0.0f) * viewMatrix().inversed() * movementSpeed * dt;
//	position += dir.xyz();
//
//	forward = forward.normalized();
//}
//
//Mat4 Camera3d::viewMatrix() const {
//	auto target = position + forward;
//	return Mat4::lookAt(position, target, up);
//}

#include <water_simulation/Camera3d.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>

void Camera3d::update(float dt) {
	Vec2 mouseOffset;
	if (lastMousePosition.has_value()) {
		mouseOffset = Input::cursorPosWindowSpace() - *lastMousePosition;
	} else {
		mouseOffset = Vec2(0.0f);
	}

	lastMousePosition = Input::cursorPosWindowSpace();

	// x+ is right both in window space and in the used coordinate system.
	// The coordinate system is left handed so by applying the left hand rule a positive angle change turns the camera right.
	angleAroundUpAxis += mouseOffset.x * rotationSpeed * dt;
	// Down is positive in window space and a positive rotation around the x axis rotates down.
	angleAroundRightAxis += mouseOffset.y * rotationSpeed * dt;

	angleAroundUpAxis = normalizeAngleZeroToTau(angleAroundUpAxis);
	angleAroundRightAxis = std::clamp(angleAroundRightAxis, -degToRad(89.0f), degToRad(89.0f));

	Vec3 movementDirection(0.0f);

	if (Input::isKeyHeld(KeyCode::A)) movementDirection += Vec3::LEFT;
	if (Input::isKeyHeld(KeyCode::D)) movementDirection += Vec3::RIGHT;
	if (Input::isKeyHeld(KeyCode::W)) movementDirection += Vec3::FORWARD;
	if (Input::isKeyHeld(KeyCode::S)) movementDirection += Vec3::BACK;
	if (Input::isKeyHeld(KeyCode::SPACE)) movementDirection += Vec3::UP;
	if (Input::isKeyHeld(KeyCode::LEFT_SHIFT)) movementDirection += Vec3::DOWN;

	movementDirection = movementDirection.normalized();

	Quat rotationAroundYAxis(angleAroundUpAxis, Vec3::UP);
	const auto dir = rotationAroundYAxis * movementDirection * movementSpeed * dt;
	position += dir;
}

Quat Camera3d::cameraForwardRotation() const {
	return Quat(angleAroundUpAxis, Vec3::UP) * Quat(angleAroundRightAxis, Vec3::RIGHT);
}

Mat4 Camera3d::viewMatrix() const {
	auto target = position + cameraForwardRotation() * Vec3::FORWARD;
	return Mat4::lookAt(position, target, Vec3::UP);
}
