#include "AudioFileStream.hpp"
#include <iostream>
#include "AudioErrorHandling.hpp"
#include <AL/al.h>

AudioFileStream AudioFileStream::make() {
    decltype(AudioFileStream::buffers) buffers;
    AL_TRY(alGenBuffers(buffers.size(), buffers.data()));

    return AudioFileStream{
        .buffers = buffers,
        .source = AudioSource::generate(),
        .state = State::STOPPED,
        .stream = nullptr,
    };
}

AudioFileStream::~AudioFileStream() {
    AL_TRY(alSourceStop(source.handle()));
    AL_TRY(alSourcei(source.handle(), AL_BUFFER, NULL));
    AL_TRY(alDeleteBuffers(buffers.size(), buffers.data()));
    stb_vorbis_close(stream);
    //AL_TRY(alSourceUnqueueBuffers((buffers.size(), buffers.data()));
}


bool AudioFileStream::useFile(const char* filename) {
    int error;
    stb_vorbis_close(stream);
    stream = stb_vorbis_open_filename(filename, &error, nullptr);
    if (stream == nullptr) {
        std::cout << "failed to load " << filename << '\n';
        return false;
    }
    return true;
}

void AudioFileStream::play() {
    if (stream == nullptr) {
        // Should this path return?
        return;
    }

    if (state == State::STOPPED) {
        for (const auto& buffer : buffers) {
            fillBuffer(buffer);
        }
        AL_TRY(alSourceQueueBuffers(source.handle(), buffers.size(), buffers.data()));
    }
    state = State::PLAYING;
    source.play();
}

void AudioFileStream::pause() {
    source.pause();
    state = State::PAUSED;
}

void AudioFileStream::stop() {
    source.stop();
    source.setBuffer(0);
    state = State::STOPPED;
}

void AudioFileStream::update() {
    if (stream == nullptr) {
        return;
    }
    if (state != State::PLAYING) {
        return;
    }

    ALint streamState;

    AL_TRY(alGetSourcei(source.handle(), AL_SOURCE_STATE, &streamState));
    if (streamState != AL_PLAYING) {
        std::cout << "stream starved";
        source.play();
        return;
    }

    ALint buffersProcessed = 0;
    AL_TRY(alGetSourcei(source.handle(), AL_BUFFERS_PROCESSED, &buffersProcessed));

    if (buffersProcessed <= 0) {
        return;
    }

    for (i32 i = 0; i < buffersProcessed; i++) {
        u32 bufferHandle;
        AL_TRY(alSourceUnqueueBuffers(source.handle(), 1, &bufferHandle));
        fillBuffer(bufferHandle);
        AL_TRY(alSourceQueueBuffers(source.handle(), 1, &bufferHandle));
    }
}

static constexpr auto BUFFER_SIZE = 1024 * 16;

void AudioFileStream::fillBuffer(u32 buffer) {
    if (stream == nullptr) {
        return;
    }

    short tempBuffer[BUFFER_SIZE];
    const auto info = stb_vorbis_get_info(stream);

    auto load = [&]() -> int {
        return stb_vorbis_get_samples_short_interleaved(
            stream,
            info.channels,
            tempBuffer,
            BUFFER_SIZE);
    };
    auto amount = load();
    if (amount == 0) {
        if (loop) {
            stb_vorbis_seek_start(stream);
            load();
        } else {
            state = State::STOPPED;
        }
    }

    // TODO: Is this correct and safe?
    const auto format = info.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    const auto size = amount * sizeof(short) * info.channels;
    AL_TRY(alBufferData(buffer, format, tempBuffer, size, info.sample_rate));
}
