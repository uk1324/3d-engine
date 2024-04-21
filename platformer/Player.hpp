#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Entities.hpp>
#include <platformer/Blocks.hpp>
#include <vector>
#include <platformer/SettingsData.hpp>

struct Player {
	Vec2 position;
	Vec2 velocity = Vec2(0.0f);

	//bool grounded = false;
	std::optional<Vec2> blockThatIsBeingStoodOnVelocity;
	bool isGrounded() const; 
	f32 elapsedSinceLastGrounded = std::numeric_limits<f32>::infinity();

	bool touchingWallOnLeft = false;
	bool touchingWallOnRight = false;

	bool jumpReleased = true;
	f32 elapsedSinceJumpPressed = std::numeric_limits<f32>::infinity();
	bool jumpedOffGround = false;

	f32 elapsedSinceLastJumped = std::numeric_limits<f32>::infinity();

	bool dead = false;

	void update();
	void updateMovement(f32 dt, std::vector<DoubleJumpOrb>& doubleJumpOrbs);
	void blockCollision(const std::vector<Block>& blocks);
	void movingBlockCollision(const std::vector<MovingBlock>& movingBlocks);
	void checkIfPlayerIsStandingOnMovingBlocks(const std::vector<MovingBlock>& movingBlocks);
	void blockCollision(
		const Aabb& playerAabb, 
		const Aabb& blockAabb, 
		BlockCollsionDirectionsBitfield collisionDirections,
		Vec2 blockVelocity);
};
