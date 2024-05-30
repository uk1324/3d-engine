#pragma once

#include <platformer/SettingsData.hpp>

struct SettingsManager {
	Settings settings;

	static Settings defaultSettings;

	void tryLoadSettings();
	void saveSettings();
};