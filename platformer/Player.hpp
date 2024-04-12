#pragma once

#include <engine/Math/Vec2.hpp>
#include <platformer/Blocks.hpp>
#include <vector>
#include <platformer/SettingsData.hpp>

struct Player {
	Vec2 position;
	Vec2 velocity;

	bool grounded = false;
	f32 elapsedSinceLastGrounded = std::numeric_limits<f32>::infinity();

	bool jumpReleased = true;
	f32 elapsedSinceJumpPressed = std::numeric_limits<f32>::infinity();

	f32 elapsedSinceLastJumped = std::numeric_limits<f32>::infinity();

	bool dead = false;

	void update();
	void updateMovement(f32 dt);
	void blockCollision(const PlayerSettings& settings, const std::vector<Block>& blocks, f32 cellSize);
};
