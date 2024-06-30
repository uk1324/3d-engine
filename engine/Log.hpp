#pragma once

#include <Put.hpp>
#include <DebugBreak.hpp>

namespace Log {
	template<typename ...Args>
	void error(const char* format, const Args&... args);

	template<typename ...Args>
	[[noreturn]] void fatal(const char* format, const Args&... args);

	// Could contatenate strings in macro.

	template<typename ...Args>
	void warning(const char* format, const Args& ...args) {
		put(std::cerr, "[ERROR] ");
		put(std::cerr, format, args...);
	}

	template<typename ...Args>
	void error(const char* format, const Args& ...args) {
		put(std::cerr, "[ERROR] ");
		put(std::cerr, format, args...);
	}

	template<typename ...Args>
	[[noreturn]] void fatal(const char* format, const Args& ...args) {
		putnn(std::cerr, "[FATAL] ");
		put(std::cerr, format, args...);
		#ifndef FINAL_RELEASE
		DEBUG_BREAK();
		#endif
		exit(EXIT_FAILURE);
	}
}

//#define LOG_WARNING(format, ...) Log::warning(format, ##__VA_ARGS__)
//#define LOG_ERROR(format, ...) Log::error(format, ##__VA_ARGS__)
//#define LOG_FATAL(format, ...) Log::fatal(format, ##__VA_ARGS__)