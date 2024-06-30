#include <platformer/SettingsManager.hpp>
#include <JsonFileIo.hpp>
#include <filesystem>
#include <fstream>
#include <engine/Input/KeyCode.hpp>

const auto settingsPath = "cached/settings.json";
const auto gameSavePath = "cached/save.json";

void SettingsManager::tryLoadSettings() {
	const auto json = tryLoadJsonFromFile(settingsPath);
	if (!json.has_value()) {
		settings = SettingsManager::defaultSettings;
		return;
	}
	try {
		settings = fromJson<Settings>(*json);
	} catch (const Json::Value::Exception&) {

	}
}

void SettingsManager::saveSettings() {
	// TODO: Maybe move the saving to a worker thread.
	std::filesystem::create_directory("./cached");
	const auto jsonSettings = toJson(settings);
	std::ofstream file(settingsPath);
	Json::print(file, jsonSettings);
}

void SettingsManager::tryLoadGameSave() {
	const auto json = tryLoadJsonFromFile(gameSavePath);
	if (!json.has_value()) {
		gameSave = GameSave{
			.roomIndex = 0
		};
		return;
	}
	try {
		gameSave = fromJson<GameSave>(*json);
	}
	catch (const Json::Value::Exception&) {

	}
}

void SettingsManager::saveGameSave() {
 	std::filesystem::create_directory("./cached");
	const auto json = toJson(gameSave);
	std::ofstream file(gameSavePath);
	Json::print(file, json);
}

Settings SettingsManager::defaultSettings = Settings{
	.audio = {
		.masterVolume = 0.5f,
		.soundEffectVolume = 0.5f,
		.musicVolume = 0.5f,
	},
	.controls = {
	/*	.left = static_cast<i32>(KeyCode::LEFT),
		.right = static_cast<i32>(KeyCode::RIGHT),
		.jump = static_cast<i32>(KeyCode::Z),
		.activate = static_cast<i32>(KeyCode::X),*/
		.left = static_cast<i32>(KeyCode::A),
		.right = static_cast<i32>(KeyCode::D),
		.jump = static_cast<i32>(KeyCode::SPACE),
		.activate = static_cast<i32>(KeyCode::J),
	}
};