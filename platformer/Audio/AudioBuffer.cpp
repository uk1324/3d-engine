#include "AudioBuffer.hpp"
#include <vector>
#include <iostream>
#include "LoadWav.hpp"
#include "AL/al.h"
#include "AudioErrorHandling.hpp"
#include "Assertions.hpp"

AudioBuffer::AudioBuffer(AudioBuffer&& other) noexcept {
    handle_ = other.handle_;
    other.handle_ = 0;
}

AudioBuffer AudioBuffer::fromFile(std::string_view path) {
    u8 channels;
    i32 sampleRate;
    u8 bitsPerSample;
    std::vector<u8> soundData;
    if (!loadWav(path, channels, sampleRate, bitsPerSample, soundData)) {
        std::cerr << "ERROR: Could not load wav" << std::endl;
        return AudioBuffer::generate();
    }

    ALenum format;
    if (channels == 1 && bitsPerSample == 8) {
        format = AL_FORMAT_MONO8;
    } else if (channels == 1 && bitsPerSample == 16) {
        format = AL_FORMAT_MONO16;
    } else if (channels == 2 && bitsPerSample == 8) {
        format = AL_FORMAT_STEREO8;
    } else if (channels == 2 && bitsPerSample == 16) {
        format = AL_FORMAT_STEREO16;
    } else {
        std::cerr
            << "ERROR: unrecognised wave format: "
            << channels << " channels, "
            << bitsPerSample << " bps" << std::endl;
        return AudioBuffer::generate();
    }

    auto buffer = AudioBuffer::generate();
    AL_TRY(alBufferData(buffer.handle(), format, soundData.data(), soundData.size(), sampleRate));
    return buffer;
}

AudioBuffer AudioBuffer::fromSize(usize sizeBytes) {
    auto buffer = AudioBuffer::generate();
    //alBufferSize
    ASSERT_NOT_REACHED();
    return buffer;
}

AudioBuffer AudioBuffer::generate() {
    u32 handle;
    AL_TRY(alGenBuffers(1, &handle));
    return AudioBuffer(handle);
}

AudioBuffer::~AudioBuffer() {
    AL_TRY(alDeleteBuffers(1, &handle_));
}

AudioBuffer::AudioBuffer(u32 handle)
    : handle_(handle) {}