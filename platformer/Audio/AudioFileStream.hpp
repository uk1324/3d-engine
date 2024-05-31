#pragma once

#include <stb_vorbis/stb_vorbis.h>
#include "AudioSource.hpp"
#include <array>

struct AudioFileStream {
	static AudioFileStream make();
	~AudioFileStream();

	bool useFile(const char* filename);
	void play();
	void pause();
	void stop();
	void update();

	void fillBuffer(u32 buffer);

	static constexpr auto BUFFER_COUNT = 4;
	std::array<u32, BUFFER_COUNT> buffers;
	AudioSource source;

	enum class State {
		PLAYING,
		PAUSED,
		STOPPED,
	};
	State state;

	bool loop = false;

	stb_vorbis* stream;
};