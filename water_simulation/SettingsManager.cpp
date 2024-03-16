#include <water_simulation/SettingsManager.hpp>
#include <FileIo.hpp>
#include <engine/Json/Json.hpp>
#include <fstream>
#include <Put.hpp>
#include <filesystem>

SettingsManager::SettingsManager()
// Creating the path at runtime because some functions change the current working directory. For example GetOpenFileNameA.
// Initializing it here to ensure the correct order of static intialization.
	: settingsPath{ (std::filesystem::current_path() / "settings.json").string() } {
	
	const auto fileStr = tryLoadStringFromFile(settingsPath.data());
	if (!fileStr.has_value()) {
		return;
	}
	try {
		const auto fileJson = Json::parse(*fileStr);
		Settings::operator=(fromJson<Settings>(fileJson));
	}
	catch (const Json::ParsingError&) {
		// else it just uses the default initialization.
	}
}

auto SettingsManager::saveToFile() const -> void {
	std::ofstream file{ settingsPath };
	Json::prettyPrint(file, toJson(*this));
	put("saving settings");
}

auto SettingsManager::saveAtScopeEnd() const -> RaiiSaver {
	return RaiiSaver{};
}

auto SettingsManager::RaiiSaver::operator->() -> SettingsManager* {
	return &SettingsManager::settings;
}

SettingsManager::RaiiSaver::~RaiiSaver() {
	SettingsManager::settings.saveToFile();
}

SettingsManager SettingsManager::settings;