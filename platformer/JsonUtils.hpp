#pragma once
#include <engine/Utils/Json.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Blocks.hpp>
#include <Array2d.hpp>

Json::Value toJson(const Aabb& aabb);
template<>
Aabb fromJson<Aabb>(const Json::Value& json);

Json::Value toJson(const Array2d<BlockType>& blockGrid);
template<>
Array2d<BlockType> fromJson<Array2d<BlockType>>(const Json::Value& json);