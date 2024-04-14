#pragma once

#include <platformer/Blocks.hpp>

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

		if (isOutOfRange || blockGrid(xd, yd) != BlockType::NORMAL) {
			collisionDirections |= direction.direction;
		}
	}

	return collisionDirections;
}

const auto SPIKE_SIZE_TO_BLOCK_SIZE_RATIO = 0.2f;

Spike makeSpikeBottom(i64 x, i64 y, f32 cellSize) {
	return Spike{ .hitbox = Aabb(
		Vec2(x, y + 1.0f - SPIKE_SIZE_TO_BLOCK_SIZE_RATIO) * cellSize, 
		Vec2(x + 1.0f, y + 1.0f) * cellSize
	)};
}

Spike makeSpikeTop(i64 x, i64 y, f32 cellSize) {
	return Spike{ .hitbox = Aabb(
		Vec2(x, y) * cellSize, 
		Vec2(x + 1.0f, y + SPIKE_SIZE_TO_BLOCK_SIZE_RATIO) * cellSize
	)};
}

Spike makeSpikeRight(i64 x, i64 y, f32 cellSize) {
	return Spike{ .hitbox = Aabb(
		Vec2(x, y) * cellSize, 
		Vec2(x + SPIKE_SIZE_TO_BLOCK_SIZE_RATIO, y + 1.0f) * cellSize
	)};
}

Spike makeSpikeLeft(i64 x, i64 y, f32 cellSize) {
	return Spike{ .hitbox = Aabb(
		Vec2(x + 1.0f - SPIKE_SIZE_TO_BLOCK_SIZE_RATIO, y) * cellSize,
		Vec2(x + 1.0f, y + 1.0f) * cellSize
	)};
}

Platform makePlatform(i64 x, i64 y, f32 cellSize) {
	return Platform{ .position = Vec2(x, y + 1) * cellSize };
}


