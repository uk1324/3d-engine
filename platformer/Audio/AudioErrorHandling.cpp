#include "AudioErrorHandling.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <iostream>
#include <Assertions.hpp>

bool checkAlError() {
    const auto error = alGetError();
    if (error == AL_NO_ERROR) {
        return false;
    }
    std::cout << "audio error\n";

    switch (error)
    {
    case ALC_INVALID_VALUE:
        std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
        break;
    case ALC_INVALID_DEVICE:
        std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
        break;
    case ALC_INVALID_CONTEXT:
        std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
        break;
    case ALC_INVALID_ENUM:
        std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
        break;
    case ALC_OUT_OF_MEMORY:
        std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
        break;
    default:
        std::cerr << "UNKNOWN ALC ERROR: " << error;
    }
    std::cerr << std::endl;

    return true;
}

void handleAudioError() {
    //std::cout << "audio error\n";
    ASSERT_NOT_REACHED();
}