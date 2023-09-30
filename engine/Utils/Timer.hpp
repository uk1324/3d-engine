#pragma once

#include <chrono>

struct Timer {
	Timer();
	float elapsedMilliseconds() const;
	float elapsedSeconds() const;
	void tookSeconds(std::string_view whatTook) const;

	std::chrono::high_resolution_clock::time_point start;
};