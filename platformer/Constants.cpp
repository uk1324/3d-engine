#include <platformer/Constants.hpp>
#include "Constants.hpp"

static GameConstants gameConstants;

const GameConstants& constants() {
	return gameConstants;
}

Aabb playerAabb(Vec2 playerPosition) {
	return Aabb(playerPosition - constants().playerSize / 2.0f, playerPosition + constants().playerSize / 2.0f);;
}

Aabb roomAabb(const LevelRoom& room) {
	return roomAabb(room.position, room.blockGrid.size());
}

Aabb roomAabb(Vec2T<i32> position, Vec2T<i64> size) {
	return Aabb(
		Vec2(position) * constants().cellSize,
		Vec2(position + Vec2T<i32>(size)) * constants().cellSize);
}
