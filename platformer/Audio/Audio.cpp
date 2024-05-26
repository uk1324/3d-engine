#include "Audio.hpp"
#include <openal-soft/include/AL/al.h>
#include "AudioErrorHandling.hpp"

Audio::Audio() {
   
}

void Audio::init() {
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

void Audio::deinit() {
    if (!alcMakeContextCurrent(nullptr)) {
        handleAudioError();
        return;
    }

    // ignore close errors
    alcDestroyContext(context);
    alcCloseDevice(device); 
}

Audio::~Audio() {
   
}

void Audio::play() {
    //const auto buffer = AudioBuffer::fromFile("./platformer/Assets/sounds/foom_0.wav");

    ////ALuint source;
    //const auto source = AudioSource::defaultInit();
    //
    //AL_TRY(alSourcei(source.handle(), AL_BUFFER, buffer.handle()));
    //AL_TRY(alSourcePlay(source.handle()));
}

void Audio::playSound(const AudioBuffer& buffer, f32 pitchMultiplier) {
    auto play = [&](AudioSource& source) {
        source.setBuffer(buffer);
        source.setPitchMultipier(pitchMultiplier);
        source.play();
    };

    for (auto& source : sources) {
        ALenum state;
        AL_TRY(alGetSourcei(source.handle(), AL_SOURCE_STATE, &state));
        if (state == AL_PLAYING || state == AL_PAUSED) {
            continue;
        }
        play(source);
        return;
    }

    sources.emplace_back(AudioSource::generate());
    auto& source = sources.back();
    play(source);
}

ALCdevice* Audio::device = nullptr;
ALCcontext* Audio::context = nullptr;