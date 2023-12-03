#pragma once

#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/Vao.hpp>
#include <framework/ShaderManager.hpp>
#include <water_simulation/Shaders/texturedQuadData.hpp>

struct ImageRenderer {
	static ImageRenderer make();

	void update();
	void drawImage(Pixel32* data, Vec2T<i64> size, const Mat3x2& transform);
	//void guiImage(Pixel32* data, Vec2T<i64> size);

	Vbo instancesVbo;
	std::vector<TexturedQuadInstance> instances;
	ShaderProgram& texturedQuadShader;
	Vbo texturedQuadVbo;
	Ibo texturedQuadIbo;
	Vao texturedQuadVao;

	struct TextureData {
		Texture texture;
		Vec2T<i64> size;
		bool drawThisFrame;
		Mat3x2 transform;
		// i64 frames since last used.
	};

	std::vector<TextureData> textures;
	std::vector<Pixel32> tempBuffer;
};
