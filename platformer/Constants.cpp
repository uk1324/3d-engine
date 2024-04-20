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
	return Aabb(
		Vec2(room.position) * constants().cellSize,
		Vec2(room.position + Vec2T<i32>(room.blockGrid.size())) * constants().cellSize);
}