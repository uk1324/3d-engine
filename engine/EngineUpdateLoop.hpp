#pragma once

#include <FixedUpdateLoop.hpp>

struct EngineUpdateLoop {
	EngineUpdateLoop(double fps);

	bool isRunning();

	FixedUpdateLoop fixedUpdateLoop;
	bool firstFrame = true;
};