#include "AudioBuffer.hpp"
#include <vector>
#include <iostream>
#include "LoadWav.hpp"
#include "AL/al.h"
#include "AudioErrorHandling.hpp"
#include <stb_vorbis/stb_vorbis.h>
#include "Assertions.hpp"

AudioBuffer::AudioBuffer(AudioBuffer&& other) noexcept {
    handle_ = other.handle_;
    other.handle_ = 0;
}

AudioBuffer AudioBuffer::fromFile(std::string_view path) {
    auto getFormat = [](int channels, int bitsPerSample) -> std::optional<ALenum> {
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
    };

    {
        u8 channels;
        i32 sampleRate;
        u8 bitsPerSample;
        std::vector<u8> soundData;
        if (loadWav(path, channels, sampleRate, bitsPerSample, soundData)) {
            const auto format = getFormat(channels, bitsPerSample);
            if (!format.has_value()) {
                return AudioBuffer::generate();
            }

            auto buffer = AudioBuffer::generate();
            AL_TRY(alBufferData(buffer.handle(), *format, soundData.data(), soundData.size(), sampleRate));
            return buffer;
        }
    }

    {
        int channels, sampleRate;
        short* output;
        const auto size = stb_vorbis_decode_filename(std::string(path).c_str(), &channels, &sampleRate, &output);
        if (size != -1) {
            auto buffer = AudioBuffer::generate();
            const auto format = getFormat(channels, 16);
            if (!format.has_value()) {
                return AudioBuffer::generate();
            }

            AL_TRY(alBufferData(buffer.handle(), *format, output, size * sizeof(u16) * channels, sampleRate));
            return buffer;
        }
    }


    std::cerr << "ERROR: Could not load audio file" << std::endl;
    return AudioBuffer::generate();
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

AudioBuffer AudioBuffer::null() {
    return AudioBuffer(0);
}

AudioBuffer& AudioBuffer::operator=(AudioBuffer&& other) noexcept {
    AL_TRY(alDeleteBuffers(1, &handle_));
    handle_ = other.handle_;
    other.handle_ = 0;
    return *this;
}

AudioBuffer::~AudioBuffer() {
    AL_TRY(alDeleteBuffers(1, &handle_));
}

AudioBuffer::AudioBuffer(u32 handle)
    : handle_(handle) {}