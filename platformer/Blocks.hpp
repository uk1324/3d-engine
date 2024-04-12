#pragma once

#include <engine/Math/Vec2.hpp>

namespace BlockCollision {
	enum Direction : u8 {
		L = 0b1000,
		R = 0b0100,
		U = 0b0010,
		D = 0b0001,
	};
}

struct Block {
	Vec2 position;
	// Exposted sides.
	u8 collisionDirections;
};

enum class BlockType : u8 {
	EMPTY,
	NORMAL,
};