#pragma once

#include "Types.hpp"
#include "AudioBuffer.hpp"
#include <optional>

struct AudioSource {
	AudioSource(AudioSource&& other) noexcept;
	static std::optional<AudioSource> tryGenerate();
	static AudioSource generate();
	static AudioSource defaultInit();
	~AudioSource();

	void setBuffer(const AudioBuffer& buffer);
	void setBuffer(u32 bufferHandle);

	void play();
	void pause();
	void stop();

	bool isPaused() const;

	void setLoop(bool loop);
	void setGain(f32 value);
	void setPitchMultipier(f32 value);

	f32 getGain();

private:
	AudioSource(u32 handle);

private:
	u32 handle_;
public:
	u32 handle() const { return handle_; }
};