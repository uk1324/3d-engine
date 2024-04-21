#include <platformer/Entities.hpp>
#include <engine/Math/Utils.hpp>
#include <platformer/Constants.hpp>
#include "Entities.hpp"

void MovingBlock::reset() {
	t = 0.0f;
	movingForward = true;
	positionDelta = Vec2(0.0f);
}

void MovingBlock::update(f32 dt) {
	const auto beforeUpdatePosition = position();
	const auto distance = startPosition.distanceTo(endPosition);
	const auto deltaT = (speed * dt) / distance;
	t += deltaT * (movingForward ? 1.0f : -1.0f);
	if (t < 0.0f) {
		movingForward = true;
		t = -t;
	} else if (t > 1.0f) {
		t = 2.0f - t;
		movingForward = false;
	}
	const auto afterUpdatePosition = position();
	positionDelta = afterUpdatePosition - beforeUpdatePosition;
}

//Vec2 MovingBlock::velocity() const {
//	return (endPosition - startPosition).normalized() * speed * (movingForward ? 1.0f : -1.0f);
//}

MovingBlock::MovingBlock(const LevelMovingBlock& movingBlock, Vec2T<i32> roomPosition)
	: startPosition(movingBlock.position + Vec2(roomPosition) * constants().cellSize)
	, endPosition(movingBlock.endPosition + Vec2(roomPosition) * constants().cellSize)
	, size(movingBlock.size)
	, speed(60.0f) {
	reset();
}

Vec2 MovingBlock::position() const {
	return lerp(startPosition, endPosition, t);
}
