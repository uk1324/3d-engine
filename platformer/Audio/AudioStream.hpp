#pragma once

#include "AudioBuffer.hpp"
#include "AudioSource.hpp"
#include <engine/Math/PerlinNoise.hpp>

struct AudioStream {
	AudioStream();

	void generateAudio(f32* output, i64 outputSize, f32 startTime, i64 samplesPerSecond);
	void fillBuffer(u32 bufferHandle);
	void play();
	void update();
	f32 currentTime = 0.0f;
	PerlinNoise noise;

	static constexpr auto BUFFER_COUNT = 4;
	static constexpr auto BUFFER_SIZE = 1024 * 32;
	//static constexpr auto BUFFER_SIZE = 44100 * 2;

	AudioBuffer buffers[BUFFER_COUNT];
	AudioSource source;
};