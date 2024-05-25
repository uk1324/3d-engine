#pragma once

#include <platformer/Audio/AudioBuffer.hpp>

struct Assets {
	AudioBuffer attractingOrbSound;
	AudioBuffer doubleJumpOrbSound;
	AudioBuffer jumpSound;
	AudioBuffer playerDeathSound;
	AudioBuffer music;
};

extern Assets* assets;

Assets loadAssets();
