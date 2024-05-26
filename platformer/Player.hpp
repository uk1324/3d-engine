#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Entities.hpp>
#include <platformer/Blocks.hpp>
#include <vector>
#include <platformer/SettingsData.hpp>
#include <platformer/GameInput.hpp>

struct Player {
	Vec2 position;
	Vec2 velocity = Vec2(0.0f);

	bool isGrounded = false;
	std::optional<Vec2> blockThatIsBeingTouchedMovementDelta;
	//bool isGrounded() const; 
	f32 elapsedSinceLastGrounded = std::numeric_limits<f32>::infinity();

	bool touchingWallOnLeft = false;
	bool touchingWallOnRight = false;

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
	//void collision(
	//	f32 dt,
	//	const std::vector<Block>& blocks, 
	//	const std::vector<Platform>& platforms,
	//	const std::vector<MovingBlock>& movingBlocks);
	//void movingBlockCollision(const std::vector<MovingBlock>& movingBlocks);
	//void checkIfPlayerIsStandingOnMovingBlocks(const std::vector<MovingBlock>& movingBlocks);
	/*void blockCollision(
		Vec2& movement,
		const Aabb& playerAabb, 
		const Aabb& blockAabb, 
		BlockCollsionDirectionsBitfield collisionDirections,
		Vec2 blockVelocity);*/
	Aabb aabb() const;
};
