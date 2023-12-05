#pragma once

#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/Vao.hpp>
#include <framework/Camera.hpp>
#include <framework/ShaderManager.hpp>
#include <framework/ShapeRenderer2d.hpp>
#include <framework/Shaders/texturedQuadData.hpp>
#include <Span2d.hpp>

struct ImageRenderer {
	static ImageRenderer make(Vbo& fullscreenQuad2dPtVerticesVbo, Ibo& fullscreenQuad2dPtVerticesIbo, Vbo& instancesVbo);

	void update(Vbo& instancesVbo);
	void drawImage(Span2d<const Pixel32> pixels, const Mat3x2& transform);
	//void guiImage(Span2d<const Pixel32> pixels);

	std::vector<TexturedQuadInstance> instances;
	ShaderProgram& texturedQuadShader;
	Vao texturedQuadVao;


	struct TextureData {
		Texture texture;
		Vec2T<i64> size;
		bool drawThisFrame;
		Mat3x2 transform;
		// i64 frames since last used.
	};

	std::vector<TextureData> textures;
};

struct Renderer2d {
	static Renderer2d make();

	void update();
	void drawImage(Span2d<const Pixel32> image, const Mat3x2& transform);
	//void drawImage(Span2d<const float> image, const Mat3x2& transform)

	ImageRenderer imageRenderer;
	ShapeRenderer2d shapeRenderer;

	Vbo fullscreenQuad2dPtVerticesVbo;
	Ibo fullscreenQuad2dPtVerticesIbo;

	Vbo instancesVbo;

	Camera camera;

	Vec2 getQuadPixelSize(Vec2 scale) const;
	float getQuadPixelSizeX(float scale) const;
	float getQuadPixelSizeY(float scale) const;

private:
	void drawDbg();
};