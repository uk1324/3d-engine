#pragma once

#include <platformer/Blocks.hpp>
#include <platformer/Constants.hpp>
#include <engine/Math/Utils.hpp>

BlockCollsionDirectionsBitfield getBlockCollisionDirections(const Array2d<BlockType>& blockGrid, i32 x, i32 y) {
	struct Entry {
		i32 x;
		i32 y;
		BlockCollsionDirectionsBitfield direction;
	};
	using namespace BlockCollisionDirections;
	const Entry directions[]{
		{.x = 1, .y = 0, .direction = R },
		{.x = -1, .y = 0, .direction = L },
		{.x = 0, .y = 1, .direction = U },
		{.x = 0, .y = -1, .direction = D },
	};

	BlockCollsionDirectionsBitfield collisionDirections = 0b0000;
	for (auto& direction : directions) {
		const i32 xd = x + direction.x;
		const i32 yd = y + direction.y;

		const bool isOutOfRange =
			xd >= blockGrid.size().x ||
			yd >= blockGrid.size().y ||
			xd < 0 ||
			yd < 0;

		if (!isOutOfRange && blockGrid(xd, yd) != BlockType::NORMAL) {
			collisionDirections |= direction.direction;
		}
	}

	return collisionDirections;
}

const auto SPIKE_SIZE_TO_BLOCK_SIZE_RATIO = 0.2f;

Spike makeSpikeBottom(i64 x, i64 y, Vec2T<i32> roomOffset) {
	return Spike{ .hitbox = Aabb(
		Vec2(x, y + 1.0f - SPIKE_SIZE_TO_BLOCK_SIZE_RATIO) * constants().cellSize + Vec2(roomOffset) * constants().cellSize,
		Vec2(x + 1.0f, y + 1.0f) * constants().cellSize + Vec2(roomOffset) * constants().cellSize
	)};
}

Spike makeSpikeTop(i64 x, i64 y, Vec2T<i32> roomOffset) {
	return Spike{ .hitbox = Aabb(
		Vec2(x, y) * constants().cellSize + Vec2(roomOffset) * constants().cellSize,
		Vec2(x + 1.0f, y + SPIKE_SIZE_TO_BLOCK_SIZE_RATIO) * constants().cellSize + Vec2(roomOffset) * constants().cellSize
	)};
}

Spike makeSpikeRight(i64 x, i64 y, Vec2T<i32> roomOffset) {
	return Spike{ .hitbox = Aabb(
		Vec2(x, y) * constants().cellSize + Vec2(roomOffset) * constants().cellSize,
		Vec2(x + SPIKE_SIZE_TO_BLOCK_SIZE_RATIO, y + 1.0f) * constants().cellSize + Vec2(roomOffset) * constants().cellSize
	)};
}

Spike makeSpikeLeft(i64 x, i64 y, Vec2T<i32> roomOffset) {
	return Spike{ .hitbox = Aabb(
		Vec2(x + 1.0f - SPIKE_SIZE_TO_BLOCK_SIZE_RATIO, y) * constants().cellSize + Vec2(roomOffset) * constants().cellSize,
		Vec2(x + 1.0f, y + 1.0f) * constants().cellSize + Vec2(roomOffset) * constants().cellSize
	)};
}

Platform makePlatform(i64 x, i64 y, Vec2T<i32> roomOffset) {
	return Platform{ .position = Vec2(x, y + 1) * constants().cellSize + Vec2(roomOffset) * constants().cellSize };
}

void DoubleJumpOrb::reset() {
	elapsedSinceUsed = std::numeric_limits<f32>::infinity();
}

bool DoubleJumpOrb::isActive() const {
	return elapsedSinceUsed > constants().doubleJumpOrbCooldown;
}

void AttractingOrb::update(bool isPlayerPressingAttractButton, f32 dt) {
	f32 speed = 1.5f;

	animationT += speed * dt * (isPlayerPressingAttractButton ? 1.0f : -1.0f);
	animationT = std::clamp(animationT, 0.0f, 1.0f);
}
