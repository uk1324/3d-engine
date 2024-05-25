#pragma once

#include <string_view>
#include <Types.hpp>

struct AudioBuffer {
	AudioBuffer(AudioBuffer&& other) noexcept;
	static AudioBuffer fromFile(std::string_view path);
	static AudioBuffer fromSize(usize sizeBytes);
	static AudioBuffer generate();
	static AudioBuffer null();

	AudioBuffer& operator=(AudioBuffer&& other) noexcept;

	~AudioBuffer();

private:
	AudioBuffer(u32 handle);

private:
	u32 handle_;
public:
	u32 handle() const { return handle_; };
};