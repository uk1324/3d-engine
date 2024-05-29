#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Entities.hpp>
#include <platformer/Blocks.hpp>
#include <vector>
#include <platformer/SettingsData.hpp>
#include <platformer/GameInput.hpp>

struct Player {
	// TODO: Maybe add some reset function.
	Vec2 position;
	Vec2 velocity = Vec2(0.0f);

	bool isGrounded = false;
	std::optional<Vec2> blockThatIsBeingTouchedMovementDelta;
	f32 elapsedSinceLastGrounded = std::numeric_limits<f32>::infinity();

	bool touchingWallOnLeft = false;
	f32 elapsedSinceTouchedWallOnLeft = std::numeric_limits<f32>::infinity();
	bool touchingWallOnRight = false;
	f32 elapsedSinceTouchedWallOnRight = std::numeric_limits<f32>::infinity();

	bool jumpReleased = true;
	f32 elapsedSinceJumpPressed = std::numeric_limits<f32>::infinity();
	bool jumpedOffGround = false;

	f32 elapsedSinceLastJumped = std::numeric_limits<f32>::infinity();

	bool dead = false;

	void updateVelocity(
		const GameInput& input,
		f32 dt, 
		std::vector<DoubleJumpOrb>& doubleJumpOrbs,
		std::vector<AttractingOrb>& attractingOrbs,
		GameAudio& audio);

	Aabb aabb() const;
};
