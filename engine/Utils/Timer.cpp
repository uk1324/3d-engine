#include "Timer.hpp"
#include "Put.hpp"
#include "Gui.hpp"

Timer::Timer()
	: start(std::chrono::high_resolution_clock::now()) {}

void Timer::reset() {
	start = std::chrono::high_resolution_clock::now();
}

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

void Timer::guiTookMiliseconds(std::string whatTook) const {
	Gui::put("% took % ms", whatTook, elapsedMilliseconds());
}
