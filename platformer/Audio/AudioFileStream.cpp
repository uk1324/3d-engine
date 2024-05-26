#include "AudioFileStream.hpp"
#include <iostream>
#include "AudioErrorHandling.hpp"
#include <AL/al.h>

AudioFileStream AudioFileStream::fromFile(const char* filename) {
    decltype(AudioFileStream::buffers) buffers;
    AL_TRY(alGenBuffers(buffers.size(), buffers.data()));

	int error;
	auto stream = stb_vorbis_open_filename(filename, &error, nullptr);
	if (stream == nullptr) {
		std::cout << "audio file stream error";
        return AudioFileStream{ 
            .buffers = buffers, 
            .source = AudioSource::generate(),
            .stream = nullptr, 
        };
	}
	const auto info = stb_vorbis_get_info(stream);

    return AudioFileStream{
        .buffers = buffers,
        .source = AudioSource::generate(),
        .stream = stream
    };
}

AudioFileStream::~AudioFileStream() {
    AL_TRY(alSourceStop(source.handle()));
    AL_TRY(alSourcei(source.handle(), AL_BUFFER, NULL));
    AL_TRY(alDeleteBuffers(buffers.size(), buffers.data()));
    //AL_TRY(alSourceUnqueueBuffers((buffers.size(), buffers.data()));
}


void AudioFileStream::play() {
    if (stream == nullptr) {
        return;
    }

    for (const auto& buffer : buffers) {
        fillBuffer(buffer);
    }
    AL_TRY(alSourceQueueBuffers(source.handle(), buffers.size(), buffers.data()));
    AL_TRY(alSourcePlay(source.handle()));
}

void AudioFileStream::update() {
    if (stream == nullptr) {
        return;
    }

    ALint state;

    AL_TRY(alGetSourcei(source.handle(), AL_SOURCE_STATE, &state));
    if (state != AL_PLAYING) {
        source.play();
        //return;
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
        stb_vorbis_seek_start(stream);
        load();
    }

    // TODO: Is this correct and safe?
    const auto format = info.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    const auto size = amount * sizeof(short) * info.channels;
    AL_TRY(alBufferData(buffer, format, tempBuffer, size, info.sample_rate));
}
