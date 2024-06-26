#include "Assets.hpp"
#include <platformer/Paths.hpp>

#define SOUND_ASSETS ASSETS_PATH "sounds"

Assets loadAssets() {
	return Assets{
		.attractingOrbSound = AudioBuffer::fromFile(SOUND_ASSETS "/hum.wav"),
		.doubleJumpOrbSound = AudioBuffer::fromFile(SOUND_ASSETS "/doubleJumpOrb.wav"),
		.jumpSound = AudioBuffer::fromFile(SOUND_ASSETS "/jump1.wav"),
		.playerDeathSound = AudioBuffer::fromFile(SOUND_ASSETS "/death.wav"),
	};
}

Assets* assets = nullptr;
