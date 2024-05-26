#pragma once

#include <platformer/Audio/AudioBuffer.hpp>
#include <platformer/Audio/AudioFileStream.hpp>
#include <vector>

struct GameAudio {
	GameAudio();

	void update();

	void playSoundEffect(const AudioBuffer& buffer, f32 pitch = 1.0f);
	void playUiSound(const AudioBuffer& buffer, f32 pitch = 1.0f);
	void play(std::vector<AudioSource>& sources, const AudioBuffer& buffer, f32 volume, f32 pitch = 1.0f);

	void updateSoundEffectVolumes();
	void updateMusicVolumes();
	void updateUiVolumes();

	void setMasterVolume(f32 value);
	void setMusicVolume(f32 value);
	void setSoundEffectVolume(f32 value);
	void setUiVolume(f32 value);
	/*struct AudioSourceData {
		AudioSource source;
		bool isAllocated;
	};*/

	void pauseSoundEffects();
	struct Source {
		AudioSource source;
		f32 volume;
	};


	AudioFileStream musicStream;

	f32 attractingOrbVolume = 0.0f;
	AudioSource attractingOrbSource;
	void setAttractingOrbVolume(f32 value);

	std::vector<AudioSource> soundEffectSources;
	std::vector<AudioSource> uiSoundSources;

private:
	f32 masterVolume = 1.0f;
	f32 soundEffectVolume = 1.0f;
	f32 musicVolume = 1.0f;
	f32 uiVolume = 1.0f;
};