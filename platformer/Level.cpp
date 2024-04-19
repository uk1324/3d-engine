#include "Level.hpp"
#include <FileIo.hpp>
#include <fstream>
#include <engine/Json/JsonParser.hpp>
#include <Put.hpp>
#include <engine/Json/JsonPrinter.hpp>

//Json::Value toJson(const Level& level) {
//	Json::Value json = Json::Value::emptyObject();
//
//	{
//		auto& blockGrid = (json["blockGrid"] = Json::Value::emptyObject());
//		auto& array = (blockGrid["array"] = Json::Value::emptyArray()).array();
//		blockGrid["sizeX"] = Json::Value(Json::Value::IntType(level.blockGrid.size().x));
//
//		for (i32 yi = 0; yi < level.blockGrid.size().y; yi++) {
//			for (i32 xi = 0; xi < level.blockGrid.size().x; xi++) {
//				array.push_back(Json::Value(Json::Value::IntType(level.blockGrid(xi, yi))));
//			}
//		}
//	}
//
//	json["levelTransitions"] = toJson<LevelTransition>(level.levelTransitions);
//
//	return json;
//}

Aabb roomAabb(const LevelRoom& room, f32 cellSize) {
	return Aabb(
		Vec2(room.position) * cellSize, 
		Vec2(room.position + Vec2T<i32>(room.blockGrid.size()))* cellSize);
}

Vec2 spawnPointToPlayerSpawnPos(
	const LevelSpawnPoint& spawnPoint,
	const PlayerSettings& settings,
	Vec2T<i32> roomPosition,
	f32 cellSize) {
	return spawnPoint.position + Vec2(roomPosition) * cellSize + settings.size / 2.0f;
}

std::optional<Level> tryLoadLevelFromFile(std::string_view path) {
	const auto text = tryLoadStringFromFile(path);
	if (!text.has_value()) {
		return std::nullopt;
	}
	try {
		const auto json = Json::parse(*text);
		return fromJson<Level>(json);
	} catch (const Json::Value::Exception&) {
		return std::nullopt;
	} catch (const Json::ParsingError&) {
		return std::nullopt;
	}
}

void saveLevelToFile(std::string_view path, const Level& level) {
	put("saving level to '%'", path);
	const auto json = toJson(level);
	std::ofstream file(path.data());
	Json::print(file, json);
}

//LevelRoom levelRoomClone(const LevelRoom& levelRoom) {
//	return LevelRoom{
//		.blockGrid = 
//	};
//}

//Vec2 levelTransitionToPlayerSpawnPos(const LevelTransition& levelTransition, const PlayerSettings& playerSettings) {
//	return levelTransition.respawnPoint + playerSettings.size / 2.0f;
//}
//
//template<>
//Level fromJson<Level>(const Json::Value& json) {
//	Level level;
//	{
//		const auto& blockGrid = json.at("blockGrid");
//		const auto& array = blockGrid.at("array").array();
//		const auto& sizeX = blockGrid.at("sizeX").intNumber();
//		if (sizeX <= 0 || sizeX > 10000) {
//			throw Json::Value::Exception();
//		}
//		const auto floatSizeY = f32(array.size()) / f32(sizeX);
//		const auto sizeY = i32(floatSizeY);
//		if (floatSizeY != f32(sizeY)) {
//			throw Json::Value::Exception();
//		}
//		// TODO: Verify the sizes.
//		level.blockGrid = Array2d<BlockType>(sizeX, sizeY);
//		for (i32 yi = 0; yi < level.blockGrid.size().y; yi++) {
//			for (i32 xi = 0; xi < level.blockGrid.size().x; xi++) {
//				const auto arrayIndex = yi * sizeX + xi;
//				// TODO: Maybe check if this is a valid type.
//				level.blockGrid(xi, yi) = BlockType(array[arrayIndex].intNumber());
//			}
//		}
//	}
//
//	level.levelTransitions = vectorFromJson<LevelTransition>(json.at("levelTransitions"));
//
//	return level;
//}
