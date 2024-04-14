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

		if (isOutOfRange || blockGrid(xd, yd) == BlockType::EMPTY) {
			collisionDirections |= direction.direction;
		}
	}

	return collisionDirections;
}
