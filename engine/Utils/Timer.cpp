#include "Timer.hpp"
#include "Put.hpp"

Timer::Timer()
	: start{ std::chrono::high_resolution_clock::now() } {}

float Timer::elapsedMilliseconds() const {
	std::chrono::duration<float, std::milli> elapsed = (std::chrono::high_resolution_clock::now() - start);
	return elapsed.count();
}

float Timer::elapsedSeconds() const {
	return elapsedMilliseconds() / 1000.0f;
}

void Timer::tookSeconds(std::string_view whatTook) const {
	put("% took % s", whatTook, elapsedSeconds());
}