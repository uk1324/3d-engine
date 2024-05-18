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

struct DoubleJumpOrb {
	Vec2 position;
	f32 elapsedSinceUsed;

	f32 animationT() const;
	void reset();
	bool isActive() const;
};

struct AttractingOrb {
	Vec2 position;
	f32 animationT = 0.0f;

	void update(bool isPlayerPressingAttractButton, f32 dt);
};

enum class BlockType : u8 {
	EMPTY,
	NORMAL,
	SPIKE_LEFT,
	SPIKE_RIGHT,
	SPIKE_TOP,
	SPIKE_BOTTOM,
	PLATFORM,
	SPIKE_TOP_RIGHT_OPEN,
	SPIKE_TOP_LEFT_OPEN,
	SPIKE_BOTTOM_RIGHT_OPEN,
	SPIKE_BOTTOM_LEFT_OPEN,
	SPIKE_TOP_RIGHT_CLOSED,
	SPIKE_TOP_LEFT_CLOSED,
	SPIKE_BOTTOM_RIGHT_CLOSED,
	SPIKE_BOTTOM_LEFT_CLOSED,
};

BlockCollsionDirectionsBitfield getBlockCollisionDirections(const Array2d<BlockType>& blockGrid, i32 x, i32 y);
Spike makeSpikeBottom(i64 x, i64 y, Vec2T<i32> roomOffset);
Spike makeSpikeTop(i64 x, i64 y, Vec2T<i32> roomOffset);
Spike makeSpikeRight(i64 x, i64 y, Vec2T<i32> roomOffset);
Spike makeSpikeLeft(i64 x, i64 y, Vec2T<i32> roomOffset);

Platform makePlatform(i64 x, i64 y, Vec2T<i32> roomOffset);