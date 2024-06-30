#pragma once

#include "Audio/Audio.hpp"
#include "Audio/AudioBuffer.hpp"
#include "Audio/AudioSource.hpp"

struct SoundGeneration {
	SoundGeneration();

	void update();

	AudioBuffer buffer;
	AudioSource source;

	//AudioBuffer buffers[2];
	i32 currentBuffer = 0;
};