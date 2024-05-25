#pragma once

#include <openal-soft/include/AL/alc.h>
#include "AudioSource.hpp"
#include <vector>

struct Audio {
	static void init();
	static void deinit();
	Audio();
	~Audio();

	void play();
	void playSound(const AudioBuffer& buffer, f32 pitchMultiplier = 1.0f);

	std::vector<AudioSource> sources;

	static ALCdevice* device;
	static ALCcontext* context;
};