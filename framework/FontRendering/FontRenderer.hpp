#pragma once

#include "Font.hpp"
#include <string_view>
#include <framework/Shaders/basicTextData.hpp>
#include <engine/Math/Vec4.hpp>
#include <engine/Graphics/Vao.hpp>
#include <framework/Camera.hpp>
#include <engine/Graphics/Ibo.hpp>

struct FontRenderer {
	FontRenderer(
		Vbo& fullscreenQuad2dPtVerticesVbo, 
		Ibo& fullscreenQuad2dPtVerticesIbo, 
		Vbo& instancesVbo);

	void render(Font& font, Vbo& instancesVbo);

	Vec2 getQuadPixelSize(const Camera& camera, Vec2 scale) const;
	float getQuadPixelSizeX(const Camera& camera, float scale) const;
	float getQuadPixelSizeY(const Camera& camera, float scale) const;

	ShaderProgram& basicTextShader;
	Vao basicTextVao;

	static Font loadFont(std::string_view directory, std::string_view fontName, std::string_view extension = "ttf");

	std::vector<BasicTextInstance> basicTextInstances;
	Vec2 addCharacterToDraw(
		const Font& font, 
		Vec2 pos, 
		const Mat3x2& transform, 
		float maxHeight, 
		char32_t character, 
		Vec4 color);
	void addTextToDraw(
		const Font& font, 
		Vec2 pos, 
		const Mat3x2& transform, 
		float maxHeight, 
		std::string_view utf8Text, 
		Vec4 color = Vec4(1.0f));
	struct TextInfo {
		Vec2 size;
		float bottomY;
	};
	TextInfo getTextInfo(const Font& font, float height, std::string_view utf8Text) const;
};