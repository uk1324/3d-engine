#include <platformer/Entities.hpp>
#include <engine/Math/Utils.hpp>
#include <platformer/Constants.hpp>
#include "Entities.hpp"
void MovingBlock::reset() {
	t = 0.0f;
	movingForward = true;
	positionDelta = Vec2(0.0f);
	if (activateOnCollision) {
		active = false;
	}
}

void MovingBlock::update(f32 dt) {
	if (!active) {
		positionDelta = Vec2(0.0f);
		return;
	}

	const auto beforeUpdatePosition = position();
	const auto distance = startPosition.distanceTo(endPosition);
	const auto deltaT = (speed * dt) / distance;
	t += deltaT * (movingForward ? 1.0f : -1.0f);
	if (t < 0.0f) {
		movingForward = true;
		t = -t;
	} else if (t > 1.0f) {
		if (stopAtEnd) {
			t = 1.0f;
		} else {
			// Go the other way.
			t = 2.0f - t;
			movingForward = false;
		}
		
	}
	const auto afterUpdatePosition = position();
	positionDelta = afterUpdatePosition - beforeUpdatePosition;
}

void MovingBlock::onPlayerCollision() {
	if (activateOnCollision) {
		active = true;
	}
}

MovingBlock::MovingBlock(const LevelMovingBlock& movingBlock, Vec2T<i32> roomPosition)
	: startPosition(movingBlock.position + Vec2(roomPosition) * constants().cellSize)
	, endPosition(movingBlock.endPosition + Vec2(roomPosition) * constants().cellSize)
	, size(movingBlock.size)
	, speed(movingBlock.speedBlockPerSecond * constants().cellSize)
	, activateOnCollision(movingBlock.activateOnCollision)
	, stopAtEnd(movingBlock.stopAtEnd) {
	if (activateOnCollision) {
		active = false;
	} else {
		active = true;
	}

	reset();
}

Vec2 MovingBlock::position() const {
	return lerp(startPosition, endPosition, smoothstep(t));
}

Aabb MovingBlock::aabb() const {
	return Aabb(position(), position() + size);
}
