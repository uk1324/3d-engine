#pragma once

#include <AL/al.h>
#include <optional>

std::optional<ALenum> getFormat(int channels, int bitsPerSample);