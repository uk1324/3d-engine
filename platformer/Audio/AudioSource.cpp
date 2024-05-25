#include "AudioSource.hpp"
#include "AudioErrorHandling.hpp"
#include <AL/al.h>

AudioSource::AudioSource(AudioSource&& other) noexcept {
    handle_ = other.handle_;
    other.handle_ = 0;
}

AudioSource AudioSource::generate() {
    u32 handle;
    AL_TRY(alGenSources(1, &handle));
    return AudioSource(handle);
}

AudioSource AudioSource::defaultInit() {
    auto source = AudioSource::generate();
    /* AL_TRY(alSourcef(source.handle(), AL_PITCH, 1));
     AL_TRY(alSourcef(source.handle(), AL_GAIN, 1.0f));
     AL_TRY(alSource3f(source.handle(), AL_POSITION, 0, 0, 0));
     AL_TRY(alSource3f(source.handle(), AL_VELOCITY, 0, 0, 0));
     AL_TRY(alSourcei(source.handle(), AL_LOOPING, AL_FALSE));*/
    return source;
}

AudioSource::~AudioSource() {
    if (handle_ == 0) {
        return;
    }
    AL_TRY(alDeleteSources(1, &handle_));
}

void AudioSource::setBuffer(const AudioBuffer& buffer) { 
    setBuffer(buffer.handle());
}

void AudioSource::setBuffer(u32 bufferHandle) {
    AL_TRY(alSourcei(handle_, AL_BUFFER, bufferHandle));
}

void AudioSource::play() {
    AL_TRY(alSourcePlay(handle_));
}

void AudioSource::setLoop(bool loop) {
    AL_TRY(alSourcei(handle_, AL_LOOPING, loop));
}

void AudioSource::setGain(f32 value) {
    AL_TRY(alSourcef(handle_, AL_GAIN, value));
}

void AudioSource::setPitchMultipier(f32 value) {
    AL_TRY(alSourcef(handle_, AL_PITCH, value));
}

f32 AudioSource::getGain() {
    ALfloat v;
    AL_TRY(alGetSourcef(handle_, AL_GAIN, &v));
    return v;
}

AudioSource::AudioSource(u32 handle)
    : handle_(handle) {}
