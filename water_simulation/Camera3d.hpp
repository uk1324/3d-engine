#include <engine/Math/Quat.hpp>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Mat4.hpp>
#include <optional>

//extern void fpsControllerUpdate(Vec2& lastMousePosition, Vec3& position, float& rotationAroundYAxis, float& rotationAroundXAxis);
//extern void fpsControllerForwardDirection(float rotationAroundYAxis, float rotationAroundXAxis);

struct Camera3d {
	std::optional<Vec2> lastMousePosition;
	Vec3 position = Vec3(0.0f);
	Vec3 forward = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

	// Angle change per pixel.
	float rotationSpeed = 0.1f;
	float movementSpeed = 10.0f;

	void update(float dt);
	Mat4 viewMatrix() const;
};