#include "GameAudio.hpp"
#include "AL/al.h"
#include <platformer/Audio/AudioErrorHandling.hpp>
#include <platformer/Assets.hpp>
#include <Put.hpp>

GameAudio::GameAudio() 
	: musicStream(AudioFileStream::make())
	, attractingOrbSource(SoundSource{ .source = AudioSource::generate() }) {

	static constexpr auto SOURCE_COUNT = 64;
	for (i32 i = 0; i < SOURCE_COUNT; i++) {
		auto source = AudioSource::tryGenerate();
		if (!source.has_value()) {
			break;
		}
		if (i % 2 == 0) {
			soundEffectSourcePool.emplace_back(SoundSource{ .source = std::move(*source) });
		} else {
			uiSoundSourcePool.emplace_back(SoundSource{ .source = std::move(*source) });
		}
	}
	for (auto& source : soundEffectSourcePool) {
		soundEffectSources.push_back(&source);
	}
	soundEffectSources.push_back(&attractingOrbSource);

	/*musicStream.useFile("./platformer/Assets/sounds/perfect-beauty.ogg");
	musicStream.play();
	musicStream.loop = true;*/

	updateSoundEffectVolumes();
}

void GameAudio::update(const SettingsAudio& settings) {
	if (masterVolume != settings.masterVolume) {
		setMasterVolume(settings.masterVolume);
	}
	if (soundEffectVolume != settings.soundEffectVolume) {
		setSoundEffectVolume(settings.soundEffectVolume);
	}
	if (musicVolume != settings.musicVolume) {
		setMusicVolume(settings.musicVolume);
	}
	musicStream.update();
}

void GameAudio::initGameAudio() {
	attractingOrbSource.source.stop();
	attractingOrbSource.source.setBuffer(assets->attractingOrbSound);
	attractingOrbSource.source.setLoop(true);
	attractingOrbSource.source.play();
	setSoundEffectSourceVolume(attractingOrbSource, 0.0f);
}

void GameAudio::playSoundEffect(const AudioBuffer& buffer, f32 pitch) {
	play(soundEffectSourcePool, buffer, soundEffectVolume, pitch);
}

void GameAudio::playUiSound(const AudioBuffer& buffer, f32 pitch) {
	//play(uiSoundSources, buffer, uiVolume, pitch);
}

void GameAudio::play(std::vector<SoundSource>& sources, const AudioBuffer& buffer, f32 volume, f32 pitch) {
	for (auto& soundSource : sources) {
		auto& source = soundSource.source;

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
	for (auto& source : soundEffectSources) {
		source->source.setGain(masterVolume * soundEffectVolume * source->volume);
	}
}

void GameAudio::updateMusicVolumes() {
	musicStream.source.setGain(masterVolume * musicVolume * musicStreamVolume);
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

void GameAudio::stopSoundEffects() {
	for (const auto& source : soundEffectSources) {
		source->source.stop();
	}
}

void GameAudio::pauseSoundEffects() {
	for (const auto& source : soundEffectSources) {
		source->source.pause();
	}
}

void GameAudio::unpauseSoundEffects() {
	for (const auto& source : soundEffectSources) {
		if (!source->source.isPaused()) {
			continue;
		}
		source->source.play();
	}
}

void GameAudio::setMusicStreamVolume(f32 volume) {
	musicStreamVolume = volume;
	updateMusicVolumes();
}

#include <imgui/imgui.h>

void GameAudio::setSoundEffectSourceVolume(SoundSource& source, f32 value) {
	source.volume = value;
	source.source.setGain(masterVolume * soundEffectVolume * value);
}
