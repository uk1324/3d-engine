#pragma once

#include <stb_vorbis/stb_vorbis.h>
#include "AudioSource.hpp"
#include <array>

struct AudioFileStream {
	static AudioFileStream fromFile(const char* filename);
	~AudioFileStream();

	void play();
	void update();

	void fillBuffer(u32 buffer);

	static constexpr auto BUFFER_COUNT = 4;
	std::array<u32, BUFFER_COUNT> buffers;

	AudioSource source;

	stb_vorbis* stream;

	//int channels;
	//int sampleRate;

	/*u32 buffers[4];
	std::string filename;*/
};