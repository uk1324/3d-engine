#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Aabb.hpp>
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

struct Spike {
	Aabb hitbox;
};

struct Platform {
	Vec2 position;
};

enum class BlockType : u8 {
	EMPTY,
	NORMAL,
	SPIKE_LEFT,
	SPIKE_RIGHT,
	SPIKE_TOP,
	SPIKE_BOTTOM,
	PLATFORM,
};

BlockCollsionDirectionsBitfield getBlockCollisionDirections(const Array2d<BlockType>& blockGrid, i32 x, i32 y);
Spike makeSpikeBottom(i64 x, i64 y, f32 cellSize);
Spike makeSpikeTop(i64 x, i64 y, f32 cellSize);
Spike makeSpikeRight(i64 x, i64 y, f32 cellSize);
Spike makeSpikeLeft(i64 x, i64 y, f32 cellSize);

Platform makePlatform(i64 x, i64 y, f32 cellSize);