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