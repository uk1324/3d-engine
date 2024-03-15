#pragma once

#include <string_view>

std::string_view trimString(std::string_view string, std::string_view whitespace = " \n\r\t");