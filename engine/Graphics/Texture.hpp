#pragma once

#include <Image32.hpp>
#include <string_view>
#include <Types.hpp>

namespace TextureSettings {

}

struct Texture {
	enum class Target {
		TEXTURE_2D = 0x0DE1,
		TEXTURE_1D = 0x0DE0,
		TEXTURE_CUBE_MAP = 0x8513,
	};

	enum class Format {
		DEPTH = 0x1902,
		DEPTH_STENCIL = 0x84F9,
		R = 0x1903,
		RG = 0x8227,
		RGB = 0x1907,
		RGBA = 0x1908,
	};

	enum class Filter {
		NEAREST = 0x2600,
		LINEAR = 0x2601
	};

	enum class Wrap {
		REPEAT = 0x2901,
		CLAMP_TO_EDGE = 0x812F,
	};

	struct Settings {
		Target target;
		Format format = Format::RGBA;
		Filter magFilter = Filter::LINEAR;
		Filter minFilter = Filter::LINEAR;
		Wrap wrapS = Wrap::REPEAT;
		Wrap wrapT = Wrap::REPEAT;
	};

	static constexpr u32 DEFAULT_TARGET = 0x0DE1; // GL_TEXTURE_2D

	Texture(uint32_t handle);
	Texture(const Image32& img, const Settings& settings);
	Texture(std::string_view path);
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator= (const Texture&) = delete;

	Texture(Texture&& other) noexcept;
	Texture& operator= (Texture&& other) noexcept;

	void bind(u32 target = DEFAULT_TARGET) const;

	u32 handle() const;

	static Texture pixelArt(const char* path);
	static Texture generate();
	static Texture null();

private:
	u32 handle_;
};
