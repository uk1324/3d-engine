#pragma once

#include <Gui.hpp>
#include <engine/Json/JsonPrinter.hpp>
#include <engine/Json/JsonParser.hpp>
#include <FileIo.hpp>
#include <fstream>

// could make a version for uniforms that only sends the uniforms if the gui was modified.
template<typename T>
struct SerializedSettings : T {
	SerializedSettings(const char* path);

	void update();

	// Could try passing the path as a template argument somehow. The address of the path shouldn't matter so this should probably work. 
	const char* path;
};

template<typename T>
SerializedSettings<T>::SerializedSettings(const char* path) 
	: path(path)
	, T([&]() {
		std::ifstream file(path);
		if (!file.good()) {
			return T();
		}
		// @Performance: also could probably do this without allocating.
		const auto string = tryLoadStringFromFile(path);
		if (!string.has_value()) {
			put("failed to load data from '%'", path);
			return T();
		}
		return fromJson<T>(Json::parse(*string));
	}()) {
}

template<typename T>
void SerializedSettings<T>::update() {
	bool changed = false;

	GUI_PROPERTY_EDITOR({
		changed = gui(*this);
	});

	if (changed) {
		// @Performance? Things like color pickers update often. Also could do this without allocating.
		const auto json = toJson(*this);
		std::ofstream file(path);
		Json::prettyPrint(file, json);
	}
}

//#define SERIALIZED_SETTINGS()