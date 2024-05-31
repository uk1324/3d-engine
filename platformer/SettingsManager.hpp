#pragma once

#include <platformer/SettingsData.hpp>

struct SettingsManager {
	Settings settings;
	GameSave gameSave;

	static Settings defaultSettings;

	void tryLoadSettings();
	void saveSettings();

	void tryLoadGameSave();
	void saveGameSave();
};