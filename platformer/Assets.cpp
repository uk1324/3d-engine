#include "Assets.hpp"

#define SOUND_ASSETS "./platformer/Assets/Sounds"

Assets loadAssets() {
	return Assets{
		.attractingOrbSound = AudioBuffer::fromFile(SOUND_ASSETS "/hum.wav"),
		.doubleJumpOrbSound = AudioBuffer::fromFile(SOUND_ASSETS "/doubleJumpOrb.wav"),
		.jumpSound = AudioBuffer::fromFile(SOUND_ASSETS "/jump1.wav"),
		.playerDeathSound = AudioBuffer::fromFile(SOUND_ASSETS "/death.wav"),
		.music = AudioBuffer::fromFile(SOUND_ASSETS "/perfect-beauty.ogg"),
	};
}

Assets* assets = nullptr;
