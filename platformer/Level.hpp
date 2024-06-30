#pragma once

#include <engine/Math/Aabb.hpp>
#include <platformer/Blocks.hpp>
#include <platformer/LevelData.hpp>
#include <Json.hpp>

static constexpr const char* FIRST_LEVEL_NAME = "";

static constexpr const char* levelsPath = "./platformer/Assets/levels/";

//Aabb roomAabb(const LevelRoom& room, f32 cellSize);

Vec2 spawnPointToPlayerSpawnPos(
	const LevelSpawnPoint& spawnPoint, 
	Vec2T<i32> roomPosition);

std::optional<Level> tryLoadLevelFromFile(std::string_view path);
void saveLevelToFile(std::string_view path, const Level& level);

struct MovingBlockAabbs {
	Aabb start, end;
};
MovingBlockAabbs movingBlockAabbs(Vec2 position, Vec2 size, Vec2 endPosition);
MovingBlockAabbs movingBlockAabbs(const LevelMovingBlock& movingBlock);


