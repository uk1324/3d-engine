#pragma once

#include <platformer/Audio/AudioBuffer.hpp>
#include <platformer/Audio/AudioFileStream.hpp>
#include <platformer/SettingsData.hpp>
#include <vector>

struct GameAudio {
	GameAudio();

	void update(const SettingsAudio& settings);

	void initGameAudio();

	struct SoundSource {
		AudioSource source;
		f32 volume = 1.0f;
	};

	void playSoundEffect(const AudioBuffer& buffer, f32 pitch = 1.0f);
	void playUiSound(const AudioBuffer& buffer, f32 pitch = 1.0f);
	void play(std::vector<SoundSource>& sources, const AudioBuffer& buffer, f32 volume, f32 pitch = 1.0f);

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

	void stopSoundEffects();
	void pauseSoundEffects();
	void unpauseSoundEffects();

	void setMusicStreamVolume(f32 volume);
	AudioFileStream musicStream;
	f32 musicStreamVolume = 1.0f;

	SoundSource attractingOrbSource;

	void setSoundEffectSourceVolume(SoundSource& source, f32 value);

	std::vector<SoundSource> soundEffectSourcePool;
	std::vector<SoundSource> uiSoundSourcePool;

	std::vector<SoundSource*> soundEffectSources;

	f32 masterVolume = 1.0f;
	f32 soundEffectVolume = 1.0f;
	f32 musicVolume = 1.0f;
	f32 uiVolume = 1.0f;
};