#pragma once

#include <engine/Math/Quat.hpp>
#include <engine/Math/Vec2.hpp>
#include <optional>

//extern void fpsControllerUpdate(Vec2& lastMousePosition, Vec3& position, float& rotationAroundYAxis, float& rotationAroundXAxis);
//extern void fpsControllerForwardDirection(float rotationAroundYAxis, float rotationAroundXAxis);

struct FpsController {
	std::optional<Vec2> lastMousePosition;
	Vec3 position = Vec3(0.0f);
	float angleAroundUpAxis = 0.0f;
	float angleAroundRightAxis = 0.0f;

	// Angle change per pixel.
	float rotationSpeed = 0.2f;
	float movementSpeed = 1.0f;

	void update(float dt);
	Quat cameraForwardRotation() const;
};