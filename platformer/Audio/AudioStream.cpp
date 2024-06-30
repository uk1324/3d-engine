#include "AudioStream.hpp"
#include "AudioErrorHandling.hpp"
#include <engine/Math/Angles.hpp>

const auto sampleRate = 44100;

AudioStream::AudioStream()
    : buffers{ AudioBuffer::generate(), AudioBuffer::generate(), AudioBuffer::generate(), AudioBuffer::generate() }
    , source(AudioSource::generate())
    , noise(PerlinNoise(0)) {

    for (i32 i = 0; i < BUFFER_COUNT; i++) {
        fillBuffer(buffers[i].handle());
    }
}

template<typename T>
T sine(T frequency) {
    return sin(frequency * TAU<T>);
}

template<typename T>
T sawtooth(T frequency) {
    return 2.0 * (fmod(frequency, 1.0) - 0.5);
}

template<typename T>
T triangle(T frequency) {
    return 2.0f * abs(frequency - floor(frequency + 0.5f));
}

void AudioStream::generateAudio(f32* output, i64 outputSize, f32 startTime, i64 samplesPerSecond) {
    const auto duration = f32(outputSize) / f32(samplesPerSecond);
    for (i64 i = 0; i < outputSize; i++) {
        const auto t = f32(i) / f32(outputSize - 1);
        const double time = startTime + duration * t;

        //const double frequency = 150.0;
        const double frequency = 200.0;
        /*const auto value = sin(TAU<f32> * frequency * time);*/
        //const double value = sin(TAU<f64> * frequency * time);
        /*const double value = sine(time * frequency) * (noise.accumulatedValue2d(Vec2(time / 5.0f, 0.0f), 4, 2.0f, 0.5f) + 0.5f) / 1.5f;*/
        /*const double value = sine(time * frequency * (noise.accumulatedValue2d(Vec2(time / 50.0f, 0.0f), 4, 2.0f, 1.0f)) + 0.1f);*/
        const double value = sine(time * frequency * (noise.accumulatedValue2d(Vec2(time / 50.0f, 0.0f), 4, 2.0f, 1.0f)) + 0.5f);
        //const double value = noise.accumulatedValue2d(Vec2(time * frequency, 0.0f), 4, 2.0f, 1.0f);
        /*const double value = triangle(time * frequency + triangle(time * 4.0f));*/
        //const double value = triangle(time * frequency) + sine(frequency);
        //const double value = noise.value2d01(Vec2(time * frequency, 0.0f));
        //const auto value = fmod(double(time) * UINT16_MAX / 100.0, 3.141592);
        //output[i] = u16((value + 1.0) / 2.0 * UINT16_MAX);
        output[i] = value;
    }
    int x = 5;
}
#include <iostream>
#include "AL/alext.h"
void AudioStream::fillBuffer(u32 bufferHandle) {
    f32 data[BUFFER_SIZE];
    generateAudio(data, BUFFER_SIZE, currentTime, sampleRate);
    /*alBufferData(bufferHandle, AL_FORMAT_MONO16, data, sizeof(data), sampleRate);*/
    alBufferData(bufferHandle, AL_FORMAT_MONO_FLOAT32, data, sizeof(data), sampleRate);
    currentTime += f32(BUFFER_SIZE) / f32(sampleRate);
}

void AudioStream::play() {
    AL_TRY(alSourceQueueBuffers(source.handle(), BUFFER_COUNT, reinterpret_cast<u32*>(&buffers[0])));
    AL_TRY(alSourcePlay(source.handle()));
}

void AudioStream::update() {
    ALint state;

    AL_TRY(alGetSourcei(source.handle(), AL_SOURCE_STATE, &state));
    if (state != AL_PLAYING) {
        return;
    }

    ALint buffersProcessed = 0;
    AL_TRY(alGetSourcei(source.handle(), AL_BUFFERS_PROCESSED, &buffersProcessed));

    if (buffersProcessed <= 0) {
        return;
    }

    u16 data[BUFFER_SIZE];
    for (i32 i = 0; i < buffersProcessed; i++) {
        u32 bufferHandle;
        AL_TRY(alSourceUnqueueBuffers(source.handle(), 1, &bufferHandle));
        fillBuffer(bufferHandle);
        AL_TRY(alSourceQueueBuffers(source.handle(), 1, &bufferHandle));
    }
}
