#pragma once

#include <engine/Math/Aabb.hpp>
#include <platformer/Blocks.hpp>
#include <platformer/LevelData.hpp>
#include <platformer/Player.hpp>
#include <Json.hpp>

struct Level {
	Array2d<BlockType> blockGrid;
	std::vector<LevelTransition> levelTransitions;
};

Json::Value toJson(const Level& level);
template<>
Level fromJson<Level>(const Json::Value& json);

static constexpr const char* FIRST_LEVEL_NAME = "";

static constexpr const char* levelsPath = "./platformer/Assets/levels/";

std::optional<Level> tryLoadLevelFromFile(std::string_view path);
void saveLevelToFile(std::string_view path, const Level& level);

Vec2 levelTransitionToPlayerSpawnPos(const LevelTransition& levelTransition, const PlayerSettings& playerSettings);