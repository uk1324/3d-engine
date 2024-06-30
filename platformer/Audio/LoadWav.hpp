#pragma once

#include <Types.hpp>
#include <string_view>
#include <vector>
#include <optional>

bool loadWav(
    std::string_view filename,
    u8& channels,
    i32& sampleRate,
    u8& bitsPerSample,
    std::vector<u8>& output);