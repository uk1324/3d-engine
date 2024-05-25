#include "Audio.hpp"
#include <openal-soft/include/AL/al.h>
#include "AudioErrorHandling.hpp"

Audio::Audio() 
    : device(nullptr)
    , context(nullptr) {
    ALCdevice* defaultDevice = alcOpenDevice(nullptr);
    if (defaultDevice == nullptr) {
        handleAudioError();
        return;
    }
    device = defaultDevice;

    context = alcCreateContext(device, nullptr);
    if (context == nullptr) {
        handleAudioError();
        return;
    }

    const auto result = alcMakeContextCurrent(context);
    if (!result) {
        handleAudioError();
        return;
    }
}

Audio::~Audio() {
    if (!alcMakeContextCurrent(nullptr)) {
        handleAudioError();
        return;
    }

    alcDestroyContext(context);
    if (checkAlError()) {
        return;
    }

    const auto result = alcCloseDevice(device); // ignore close errors
}

void Audio::play() {
    //const auto buffer = AudioBuffer::fromFile("./platformer/Assets/sounds/foom_0.wav");

    ////ALuint source;
    //const auto source = AudioSource::defaultInit();
    //
    //AL_TRY(alSourcei(source.handle(), AL_BUFFER, buffer.handle()));
    //AL_TRY(alSourcePlay(source.handle()));
}

