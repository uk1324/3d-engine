#include "SoundGeneration.hpp"
#include <engine/Input/Input.hpp>
#include <AL/al.h>
#include "Audio/AudioErrorHandling.hpp"

SoundGeneration::SoundGeneration() 
	: source(AudioSource::defaultInit())
	, buffer(AudioBuffer::fromFile("./platformer/Assets/sounds/hum.wav")) {
}

void SoundGeneration::update() {
	if (Input::isKeyDown(KeyCode::L)) {
		AL_TRY(alSourcei(source.handle(), AL_BUFFER, buffer.handle()));
		AL_TRY(alSourcePlay(source.handle()));
	}
}
