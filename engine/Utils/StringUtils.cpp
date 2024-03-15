#include "StringUtils.hpp"

std::string_view trimString(std::string_view string, std::string_view whitespace) {
	auto start = string.find_first_not_of(whitespace);
	if (start == std::string_view::npos) {
		start = 0;
	}
	auto end = string.find_last_not_of(whitespace);
	if (end == std::string_view::npos) {
		end = string.size();
	}
	return string.substr(start, end - start + 1);
}
