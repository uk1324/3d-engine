#pragma once

#include <chrono>

struct Timer {
	Timer();
	void reset();

	float elapsedMilliseconds() const;
	float elapsedSeconds() const;
	void tookSeconds(std::string_view whatTook) const;
	void guiTookMiliseconds(std::string whatTook) const;

	std::chrono::high_resolution_clock::time_point start;
};