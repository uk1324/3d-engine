#pragma once
#include <engine/Utils/Json.hpp>
#include <engine/Math/Aabb.hpp>

Json::Value toJson(const Aabb& aabb);
template<>
Aabb fromJson<Aabb>(const Json::Value& json);