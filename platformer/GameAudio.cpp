#include "GameAudio.hpp"
#include "AL/al.h"
#include <platformer/Audio/AudioErrorHandling.hpp>
#include <platformer/Assets.hpp>
#include <Put.hpp>

GameAudio::GameAudio() 
	: musicStream(AudioFileStream::fromFile("./platformer/Assets/sounds/perfect-beauty.ogg"))
	, attractingOrbSource(AudioSource::generate()) {

	static constexpr auto SOURCE_COUNT = 64;
	for (i32 i = 0; i < SOURCE_COUNT; i++) {
		auto source = AudioSource::tryGenerate();
		if (!source.has_value()) {
			break;
		}
		if (i % 2 == 0) {
			soundEffectSources.push_back(std::move(*source));
		} else {
			uiSoundSources.push_back(std::move(*source));
		}
	}

	attractingOrbSource.setBuffer(assets->attractingOrbSound);
	attractingOrbSource.play();
	attractingOrbSource.setLoop(true);

	updateSoundEffectVolumes();

	musicStream.play();
}

void GameAudio::update() {
	musicStream.update();
}

void GameAudio::playSoundEffect(const AudioBuffer& buffer, f32 pitch) {
	play(soundEffectSources, buffer, soundEffectVolume, pitch);
}

void GameAudio::playUiSound(const AudioBuffer& buffer, f32 pitch) {
	play(uiSoundSources, buffer, uiVolume, pitch);
}

void GameAudio::play(std::vector<AudioSource>& sources, const AudioBuffer& buffer, f32 volume, f32 pitch) {
	for (auto& source : sources) {
		ALenum state;
		AL_TRY(alGetSourcei(source.handle(), AL_SOURCE_STATE, &state));
		if (state == AL_PLAYING || state == AL_PAUSED) {
			continue;
		}
		{
			source.setBuffer(buffer);
			source.setPitchMultipier(pitch);
			source.setGain(masterVolume * volume);
			source.play();
		}
		return;
	}
	put("run out of sources");

}

void GameAudio::updateSoundEffectVolumes() {
	const auto volume = masterVolume * soundEffectVolume;

	for (auto& source : soundEffectSources) {
		source.setGain(volume);
	}
	setAttractingOrbVolume(attractingOrbVolume);
}

void GameAudio::updateMusicVolumes() {
	const auto volume = masterVolume * musicVolume;
	musicStream.source.setGain(volume);
}

void GameAudio::updateUiVolumes() {
}

void GameAudio::setMasterVolume(f32 value) {
	masterVolume = value;
	updateMusicVolumes();
	updateSoundEffectVolumes();
	updateUiVolumes();
}

void GameAudio::setMusicVolume(f32 value) {
	musicVolume = value;
	updateMusicVolumes();
}

void GameAudio::setSoundEffectVolume(f32 value) {
	soundEffectVolume = value;
	updateSoundEffectVolumes();
}

void GameAudio::setUiVolume(f32 value) {
	uiVolume = value;
	updateUiVolumes();
}

void GameAudio::pauseSoundEffects() {
	/*alSourcePause(attractingOrbSource.handle());*/
	alSourceStop(attractingOrbSource.handle());
	alSourceStop(musicStream.source.handle());
}

void GameAudio::setAttractingOrbVolume(f32 value) {
	attractingOrbVolume = value;
	attractingOrbSource.setGain(masterVolume * soundEffectVolume * attractingOrbVolume);
}
