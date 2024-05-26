#include "AudioFormat.hpp"
#include <iostream>

std::optional<ALenum> getFormat(int channels, int bitsPerSample) {
    if (channels == 1 && bitsPerSample == 8) {
        return AL_FORMAT_MONO8;
    } else if (channels == 1 && bitsPerSample == 16) {
        return AL_FORMAT_MONO16;
    } else if (channels == 2 && bitsPerSample == 8) {
        return AL_FORMAT_STEREO8;
    } else if (channels == 2 && bitsPerSample == 16) {
        return AL_FORMAT_STEREO16;
    } else {
        std::cerr
            << "ERROR: unrecognised audio format: "
            << channels << " channels, "
            << bitsPerSample << " bps" << std::endl;
        return std::nullopt;
    }
}