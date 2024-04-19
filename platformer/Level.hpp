#pragma once

#include <engine/Math/Aabb.hpp>
#include <platformer/Blocks.hpp>
#include <platformer/LevelData.hpp>
#include <platformer/Player.hpp>
#include <Json.hpp>

static constexpr const char* FIRST_LEVEL_NAME = "";

static constexpr const char* levelsPath = "./platformer/Assets/levels/";

Aabb roomAabb(const LevelRoom& room, f32 cellSize);

Vec2 spawnPointToPlayerSpawnPos(
	const LevelSpawnPoint& spawnPoint, 
	const PlayerSettings& settings,
	Vec2T<i32> roomPosition,
	f32 cellSize);

std::optional<Level> tryLoadLevelFromFile(std::string_view path);
void saveLevelToFile(std::string_view path, const Level& level);

LevelRoom levelRoomClone(const LevelRoom& levelRoom);
