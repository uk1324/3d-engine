#include "FixedUpdateLoop.hpp"
#include <chrono>

static double currentTime() {
	using namespace std::chrono;
	return duration<double>(steady_clock::now().time_since_epoch()).count();
};

FixedUpdateLoop::FixedUpdateLoop(double fps)
	: updateFrameTime(1.0 / fps)
	, lastFrameStartTime(currentTime()) {
}

bool FixedUpdateLoop::isRunning() {
	constexpr double FRAME_TIME_CAP = 2.0;

	while (accumulatedTime < updateFrameTime) {
		frameStartTime = currentTime();
		frameTime = frameStartTime - lastFrameStartTime;
		lastFrameStartTime = frameStartTime;

		if (frameTime > FRAME_TIME_CAP)
			frameTime = FRAME_TIME_CAP;

		accumulatedTime += frameTime;
	}
	accumulatedTime -= updateFrameTime;
	return running;
}
