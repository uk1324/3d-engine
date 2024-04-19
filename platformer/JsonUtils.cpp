#include <platformer/JsonUtils.hpp>

Json::Value toJson(const Aabb& aabb) {
	auto json = Json::Value::emptyObject();
	json["min"] = toJson(aabb.min);
	json["max"] = toJson(aabb.max);
	return json;
}

template<>
Aabb fromJson<Aabb>(const Json::Value& json) {
	return Aabb(fromJson<Vec2>(json.at("min")), fromJson<Vec2>(json.at("max")));
}

Json::Value toJson(const Array2d<BlockType>& blockGrid) {
	auto blockGridJson = Json::Value::emptyObject();
	auto& array = (blockGridJson["array"] = Json::Value::emptyArray()).array();
	blockGridJson["sizeX"] = Json::Value(Json::Value::IntType(blockGrid.size().x));

	for (i32 yi = 0; yi < blockGrid.size().y; yi++) {
		for (i32 xi = 0; xi < blockGrid.size().x; xi++) {
			array.push_back(Json::Value(Json::Value::IntType(blockGrid(xi, yi))));
		}
	}
	return blockGridJson;
}

template<>
Array2d<BlockType> fromJson<Array2d<BlockType>>(const Json::Value& json) {
	const auto& array = json.at("array").array();
	const auto& sizeX = json.at("sizeX").intNumber();
	if (sizeX <= 0 || sizeX > 10000) {
		throw Json::Value::Exception();
	}
	const auto floatSizeY = f32(array.size()) / f32(sizeX);
	const auto sizeY = i32(floatSizeY);
	if (floatSizeY != f32(sizeY)) {
		throw Json::Value::Exception();
	}
	// TODO: Verify the sizes.
	Array2d<BlockType> result = Array2d<BlockType>(sizeX, sizeY);
	for (i32 yi = 0; yi < result.size().y; yi++) {
		for (i32 xi = 0; xi < result.size().x; xi++) {
			const auto arrayIndex = yi * sizeX + xi;
			// TODO: Maybe check if this is a valid type.
			result(xi, yi) = BlockType(array[arrayIndex].intNumber());
		}
	}
	return result;
}