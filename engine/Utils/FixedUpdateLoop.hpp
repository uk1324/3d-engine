#pragma once

struct FixedUpdateLoop {
	FixedUpdateLoop(double fps);

	bool isRunning();

	bool running = true;

private:
	// Maybe use integers to store the time. This might have some issues with floating point precistion.
	
	// Initialized to fix warnings.
	double lastFrameStartTime = 0.0;
	double frameStartTime = 0.0;
	double frameTime = 0.0;

	double accumulatedTime = 0.0;
	double updateFrameTime;

};