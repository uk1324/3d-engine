#pragma once

#include <platformer/Level.hpp>

struct MovingBlock {
	MovingBlock(const LevelMovingBlock& movingBlock, Vec2T<i32> roomPosition);
	void reset();
	void update(f32 dt);
	void onPlayerCollision();
	//Vec2 velocity() const;

	Vec2 position() const;
	Aabb aabb() const;

	Vec2 startPosition;
	Vec2 endPosition;
	Vec2 size;
	f32 t;
	f32 speed;
	Vec2 positionDelta;
	bool movingForward;
	bool active;
	bool activateOnCollision;
	bool stopAtEnd;
};