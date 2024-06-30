#pragma once

#include <engine/Json/Json.hpp>
#include <string_view>
#include <optional>

Json::Value jsonFromFile(std::string_view path);
std::optional<Json::Value> tryLoadJsonFromFile(std::string_view path);