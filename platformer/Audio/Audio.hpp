#pragma once

#include <openal-soft/include/AL/alc.h>

struct Audio {
	Audio();
	~Audio();

	void play();

	ALCdevice* device;
	ALCcontext* context;
};