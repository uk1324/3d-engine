#pragma once

#include "Types.hpp"

struct AudioSource {
	AudioSource(AudioSource&& other) noexcept;
	static AudioSource generate();
	static AudioSource defaultInit();
	~AudioSource();

private:
	AudioSource(u32 handle);

private:
	u32 handle_;
public:
	u32 handle() const { return handle_; }
};