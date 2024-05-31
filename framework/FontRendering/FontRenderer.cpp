#include "FontRenderer.hpp"
#include <glad/glad.h>
#include <engine/Utils/Utf8.hpp>
#include <framework/Instancing.hpp>
#include <framework/ShaderManager.hpp>
#include <framework/Renderer2d.hpp>
#include <engine/Window.hpp>
#include <engine/Log.hpp>

FontRenderer::FontRenderer(
	Vbo& fullscreenQuad2dPtVerticesVbo, 
	Ibo& fullscreenQuad2dPtVerticesIbo, 
	Vbo& instancesVbo) 
	: basicTextShader(MAKE_GENERATED_SHADER(BASIC_TEXT))
	, basicTextVao(createInstancingVao<BasicTextShader>(fullscreenQuad2dPtVerticesVbo, fullscreenQuad2dPtVerticesIbo, instancesVbo)) {
}

void FontRenderer::render(Font& font, Vbo& instancesVbo) {
	basicTextShader.use();
	basicTextShader.setTexture("fontAtlas", 0, font.fontAtlas);
	drawInstances(basicTextVao, instancesVbo, basicTextInstances, Renderer2d::drawFullscreenQuad2dInstances);
	basicTextInstances.clear();
}

Vec2 FontRenderer::getQuadPixelSize(const Camera& camera, Vec2 scale) const {
	return Vec2(getQuadPixelSizeX(camera, scale.x), getQuadPixelSizeX(camera, scale.y));
}

float FontRenderer::getQuadPixelSizeX(const Camera& camera, float scale) const {
	return scale * camera.zoom * Window::size().x;
}

float FontRenderer::getQuadPixelSizeY(const Camera& camera, float scale) const {
	return scale * camera.zoom * Window::size().y;
}

Font FontRenderer::loadFont(std::string_view directory, std::string_view fontName, std::string_view extension) {
	//Timer timer;
	const auto fontPath = std::string(directory) + "/" + std::string(fontName) + "." + std::string(extension);
	const auto atlasPath = "cached/" + std::string(fontName) + ".png";
	const auto infoPath = "cached/" + std::string(fontName) + ".json";

	auto font = fontLoadSdfWithCaching(
		fontPath.c_str(),
		atlasPath.c_str(),
		infoPath.c_str(),
		ASCII_CHARACTER_RANGES,
		64
	);
	//put("font loading took %", timer.elapsedMilliseconds());
	if (font.has_value()) {
		return std::move(*font);
	}
	LOG_FATAL("failed to load font: \n %s", font.error().message.c_str());
}

Vec2 FontRenderer::addCharacterToDraw(
	const Font& font, 
	Vec2 pos, 
	const Mat3x2& transform, 
	float maxHeight, 
	char32_t character, 
	Vec4 color) {
	const auto& characterIt = font.glyphs.find(character);
	if (characterIt == font.glyphs.end()) {
		return pos;
	}
	const auto& info = characterIt->second;

	float scale = maxHeight * (1.0f / font.pixelHeight);
	auto centerPos = pos + Vec2(info.bearingRelativeToOffsetInAtlas.x, -(info.sizeInAtlas.y - info.bearingRelativeToOffsetInAtlas.y)) * scale;
	const auto size = Vec2(info.sizeInAtlas) * scale;
	centerPos += size / 2.0f;

	if (info.isVisible()) {
		const auto characterTransform = makeObjectTransform(centerPos, 0.0f, size / 2.0f) * transform;
		//const auto scaleTransform = (objectTransform(Vec2(0.0f), 0.0f, Vec2(0.0f, maxHeight)) * transform).removedTranslation();
		// Using size.y for the scale pixel size calculation instead of max height seems more correct, because the all objects should have more similar smoothing then I think.
		const auto scaleTransform = (makeObjectTransform(Vec2(0.0f), 0.0f, Vec2(0.0f, size.y)) * transform).removedTranslation();
		const auto pixelSize = (Vec2(1.0f) * scaleTransform * Window::size()).y;
		basicTextInstances.push_back(BasicTextInstance{
			.transform = characterTransform,
			.offsetInAtlas = Vec2(info.offsetInAtlas) / Vec2(font.fontAtlasPixelSize),
			.sizeInAtlas = Vec2(info.sizeInAtlas) / Vec2(font.fontAtlasPixelSize),
			// This value is incorrect because it uses pixel size of the quad and not the size of the sdf outline. This looks good enough, but might vary between fonts.
			.smoothing = 15.0f / pixelSize,
			.color = color
		});
	}

	// Advance values are stored as 1/64 of a pixel.
	return Vec2(pos.x + (info.advance.x >> 6) * scale, pos.y);
}

void FontRenderer::addTextToDraw(
	const Font& font, 
	Vec2 pos, 
	const Mat3x2& transform, 
	float maxHeight, 
	std::string_view utf8Text, 
	Vec4 color) {
	const char* current = utf8Text.data();
	const char* end = utf8Text.data() + utf8Text.size();
	while (const auto codepoint = Utf8::readCodePoint(current, end)) {
		current = codepoint->next;
		pos = addCharacterToDraw(font, pos, transform, maxHeight, codepoint->codePoint, color);
	}
}

FontRenderer::TextInfo FontRenderer::getTextInfo(const Font& font, float maxHeight, std::string_view utf8Text) const {
	const auto scale = maxHeight * (1.0f / font.pixelHeight);

	float width = 0.0f;
	float minY = std::numeric_limits<float>::infinity();
	float maxY = 0.0f;

	const char* current = utf8Text.data();
	const char* end = utf8Text.data() + utf8Text.size();
	while (const auto codepoint = Utf8::readCodePoint(current, end)) {
		current = codepoint->next;
		const auto& characterIt = font.glyphs.find(codepoint->codePoint);
		if (characterIt == font.glyphs.end()) {
			continue;
		}
		const auto& info = characterIt->second;
		const auto advance = (info.advance.x >> 6) * scale;
		width += advance;
		const auto bottomY = -(info.visibleSize.y - info.visibleBearing.y);
		const auto topY = bottomY + info.visibleSize.y;
		minY = std::min(minY, bottomY * scale);
		maxY = std::max(maxY, topY * scale);
	}
	// Don't have only add width istead of advance if it is the last character, because of how advance works.
	// In the glyph metrics image you can see that the character is offset from the origin.
	// https://learnopengl.com/In-Practice/Text-Rendering
	// This might not be correct, but I think it is.
	// You can test it works by for example writing "oo"

	float height = maxY - minY;
	if (isnan(height)) {
		height = 0.0f;
	}

	return TextInfo{
		.size = Vec2(width, height),
		.bottomY = minY
	};
}
