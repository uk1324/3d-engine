#pragma once

#include <platformer/Level.hpp>

struct MovingBlock {
	MovingBlock(const LevelMovingBlock& movingBlock, Vec2T<i32> roomPosition);
	void reset();
	void update(f32 dt);

	Vec2 position() const;

	Vec2 startPosition;
	Vec2 endPosition;
	Vec2 size;
	f32 t;
	f32 speed;
	bool movingForward;
};