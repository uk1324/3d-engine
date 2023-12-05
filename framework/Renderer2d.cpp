#include <framework/Renderer2d.hpp>
#include <framework/Instancing.hpp>
#include <framework/FullscrenQuadPt.hpp>
#include <framework/Dbg.hpp>
#include <StructUtils.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>

ImageRenderer ImageRenderer::make(Vbo& fullscreenQuad2dPtVerticesVbo, Ibo& fullscreenQuad2dPtVerticesIbo, Vbo& instancesVbo) {
	auto texturedQuadVao = Vao::generate();
	TexturedQuadShader::addAttributesToVao(texturedQuadVao, fullscreenQuad2dPtVerticesVbo, instancesVbo);
	texturedQuadVao.bind();
	fullscreenQuad2dPtVerticesIbo.bind();
	Vao::unbind();
	Ibo::unbind();

	auto value = ImageRenderer{
		.texturedQuadShader = MAKE_GENERATED_SHADER(TEXTURED_QUAD),
		MOVE(texturedQuadVao),
	};
	return value;
}

void ImageRenderer::update(Vbo& instancesVbo) {
	texturedQuadShader.use();
	// @Performance: Binding multiple textures and put the index into an instance id. UBO? Is there a better way?
	// What about samplers?
	for (auto& texture : textures) {
		if (!texture.drawThisFrame) {
			continue;
		}
		
		instances.push_back(TexturedQuadInstance{
			.transform = texture.transform,
		});
		texturedQuadShader.setTexture("renderedTexture", 0, texture.texture);
		// @Performance: Binding.
		drawInstances(texturedQuadVao, instancesVbo, instances, [](usize instanceCount) {
			glDrawElementsInstanced(GL_TRIANGLES, std::size(fullscreenQuad2dPtIndices), GL_UNSIGNED_INT, nullptr, instanceCount);
		});
		instances.clear();
	}

	for (auto& texture : textures) {
		texture.drawThisFrame = false;
	}
}

void ImageRenderer::drawImage(Span2d<const Pixel32> data, const Mat3x2& transform) {
	TextureData* textureToSaveTo = nullptr;

	for (auto& texture : textures) {
		if (texture.size == data.size()) {
			textureToSaveTo = &texture;
		}
	}

	static constexpr auto TYPE = GL_UNSIGNED_BYTE;
	static constexpr auto FORMAT = GL_RGBA;
	static constexpr auto INTERNAL_FORMAT = GL_RGBA8;

	if (textureToSaveTo == nullptr) {
		auto texture = Texture::generate();

		texture.bind();
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			INTERNAL_FORMAT,
			data.sizeX(),
			data.sizeY(),
			0,
			FORMAT,
			TYPE,
			nullptr
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		textures.push_back(TextureData{
			MOVE(texture),
			.size = data.size(),
		});
		textureToSaveTo = &textures.back();
	}
	
	textureToSaveTo->texture.bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.sizeX(), data.sizeY(), FORMAT, TYPE, data.data());
	textureToSaveTo->drawThisFrame = true;
	textureToSaveTo->transform = transform;
}

Renderer2d Renderer2d::make() {
	auto fullscreenQuad2dPtVerticesVbo = Vbo(fullscreenQuad2dPtVertices, sizeof(fullscreenQuad2dPtVertices));
	auto fullscreenQuad2dPtVerticesIbo = Ibo(fullscreenQuad2dPtIndices, sizeof(fullscreenQuad2dPtIndices));
	auto instancesVbo = Vbo(4 * 1024ull);

	auto imageRenderer = ImageRenderer::make(fullscreenQuad2dPtVerticesVbo, fullscreenQuad2dPtVerticesIbo, instancesVbo);
	auto shapeRenderer = ShapeRenderer2d::make(fullscreenQuad2dPtVerticesVbo, fullscreenQuad2dPtVerticesIbo, instancesVbo);

	return Renderer2d{
		MOVE(imageRenderer),
		MOVE(shapeRenderer),
		MOVE(fullscreenQuad2dPtVerticesVbo),
		MOVE(fullscreenQuad2dPtVerticesIbo),
		MOVE(instancesVbo)
	};
}

void Renderer2d::update() {
	// This probably should be put into an another update that gets called before drawing so that the camera has the correct state before creating transforms.
	camera.aspectRatio = Window::aspectRatio();

	drawDbg();

	imageRenderer.update(instancesVbo);
	shapeRenderer.update(instancesVbo);
}

void Renderer2d::drawImage(Span2d<const Pixel32> image, const Mat3x2& transform) {
	imageRenderer.drawImage(image, transform);
}

Vec2 Renderer2d::getQuadPixelSize(Vec2 scale) const {
	return Vec2(getQuadPixelSizeX(scale.x), getQuadPixelSizeX(scale.y));
}

float Renderer2d::getQuadPixelSizeX(float scale) const {
	return scale * camera.zoom * Window::size().x;
}

float Renderer2d::getQuadPixelSizeY(float scale) const {
	return scale * camera.zoom * Window::size().y;
}

void Renderer2d::drawDbg() {
	// TODO: How to properly handle transparency and rendering the instances object in order. Just using the depth buffer won't work, because then the transparent parts overrite the z buffer. Would either need to sort or do addative blending and discard the depth value on non opaque pixels.
	auto calculateDepth = [](i32 drawIndex) -> float {
		return -drawIndex / static_cast<float>(Dbg::drawnThingsCount);
	};

	// @Performance maybe flip the order to avoid overdraw.

	for (i32 i = 0; i < Dbg::drawnThingsCount; i++) {

	}

	for (const auto& disk : Dbg::disks) {
		const auto pixelSize = getQuadPixelSizeY(disk.radius);
		shapeRenderer.diskInstances.push_back(DiskInstance{
			.transform = camera.makeTransform(disk.pos, 0.0f, Vec2(disk.radius)),
			.color = Vec4(disk.color, 1.0f),
			.smoothing = 5.0f / pixelSize,
			.depth = calculateDepth(disk.drawIndex)
		});
	}

	for (const auto& line : Dbg::lines) {
		const auto vector = line.end - line.start;
		const auto direction = vector.normalized();
		const auto width = line.width.has_value() ? *line.width : 20.0f / Window::size().y;
		const auto pixelWidth = getQuadPixelSizeY(width);
		shapeRenderer.lineInstances.push_back(LineInstance{
			.transform = camera.makeTransform(line.start + vector / 2.0f, vector.angle(), Vec2(vector.length() / 2.0f + width, width)),
			.color = Vec4(line.color, 1.0f),
			.smoothing = 3.0f / pixelWidth,
			.lineWidth = width,
			.lineLength = vector.length() + width * 2.0f,
			.depth = calculateDepth(line.drawIndex)
		});
	}

	Dbg::update();
}
