#pragma once

#include <engine/Math/Vec2.hpp>
#include <Array2d.hpp>

namespace BlockCollisionDirections {
	enum {
		L = 0b1000,
		R = 0b0100,
		U = 0b0010,
		D = 0b0001,
	};
}

using BlockCollsionDirectionsBitfield = u8;

struct Block {
	Vec2 position;
	// Exposted sides.
	BlockCollsionDirectionsBitfield collisionDirections;
};

enum class BlockType : u8 {
	EMPTY,
	NORMAL,
};

BlockCollsionDirectionsBitfield getBlockCollisionDirections(const Array2d<BlockType>& blockGrid, i32 x, i32 y);