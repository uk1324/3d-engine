#include "JsonFileIo.hpp"
#include "FileIo.hpp"
#include <engine/Log.hpp>

Json::Value jsonFromFile(std::string_view path) {
	auto optJson = tryLoadJsonFromFile(path);
	if (optJson.has_value()) {
		return std::move(*optJson);
	}
	LOG_FATAL("couldn't parse json file \"%s\"", path.data());
	return Json::Value::null();
}

std::optional<Json::Value> tryLoadJsonFromFile(std::string_view path) {
	const auto string = tryLoadStringFromFile(path);
	if (!string.has_value()) {
		return std::nullopt;
	}
	try {
		return Json::parse(*string);
	} catch (const Json::ParsingError&) {
		return std::nullopt;
	}
}
