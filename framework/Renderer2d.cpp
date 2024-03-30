#include <framework/Renderer2d.hpp>
#include <framework/Instancing.hpp>
#include <framework/FullscrenQuadPt.hpp>
#include <framework/Dbg.hpp>
#include <StructUtils.hpp>
#include <FileIo.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <framework/Shaders/fullscreenQuadData.hpp>

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
		// This causes GL_INVALID_ENUM when called on laptop.
		/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);*/
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

	Vao fullscreenQuad2dPtVerticesVao = Vao::generate();
	FullscreenQuadShader::addAttributesToVao(
		fullscreenQuad2dPtVerticesVao,
		fullscreenQuad2dPtVerticesVbo,
		instancesVbo);
	fullscreenQuad2dPtVerticesVao.bind();
	fullscreenQuad2dPtVerticesIbo.bind();
	Vao::unbind();

	auto source = tryLoadStringFromFile("framework/Shaders/fullscreenQuad.vert");
	std::string fullscreenQuadVertSource;
	if (source.has_value()) {
		fullscreenQuadVertSource = std::move(*source);
	} else {
		ASSERT_NOT_REACHED();
	}

	return Renderer2d{
		MOVE(imageRenderer),
		MOVE(shapeRenderer),
		MOVE(fullscreenQuad2dPtVerticesVbo),
		MOVE(fullscreenQuad2dPtVerticesIbo),
		MOVE(fullscreenQuad2dPtVerticesVao),
		MOVE(instancesVbo),
		MOVE(fullscreenQuadVertSource),
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
	/*auto calculateDepth = [](i32 drawIndex) -> float {
		return -drawIndex / static_cast<float>(Dbg::drawnThingsCount);
	};*/

	// @Performance maybe flip the order to avoid overdraw.

	/* 
	Problem: Rendering 2d shapes in the correct order and correctly blended.

	Rendering in correct order:
	- Use depth buffer
	- Render sorted

	Correctly blending:
	- Render in correct order.
	- It would probably be possible to render the opaque parts of objects first and then use sorme sort of depth peeling only on the transparent parts of objects.

	Sorting and then rendering everything using a single shader.
	Issues:
	- Have to pass the data to each instance. Could just pass the type of rendered object and an index to an array containing instances of that type. Another option would to to use a union type, but decoding it inside of the shader would probably be a pain.
	- Branching in shaders is inefficient.
	*/

	const auto defaultWidth = 20.0f / Window::size().y;
	// TODO: Could try adding the smoothing to the size so the actual size without the smoothing is solid.

	for (const auto& disk : Dbg::disks) {
		const auto pixelSize = getQuadPixelSizeY(disk.radius);
		shapeRenderer.diskInstances.push_back(DiskInstance{
			.transform = camera.makeTransform(disk.pos, 0.0f, Vec2(disk.radius)),
			.color = Vec4(disk.color, 1.0f),
			.smoothing = 5.0f / pixelSize,
		});
	}

	for (const auto& circle : Dbg::circles) {
		const auto pixelSize = getQuadPixelSizeY(circle.radius);
		shapeRenderer.circleInstances.push_back(CircleInstance{
			.transform = camera.makeTransform(circle.pos, 0.0f, Vec2(circle.radius)),
			.color = Vec4(circle.color, 1.0f),
			.smoothing = 5.0f / pixelSize,
			.width = circle.width.value_or(defaultWidth)
		});
	}

	for (const auto& line : Dbg::lines) {
		const auto vector = line.end - line.start;
		const auto direction = vector.normalized();
		/*const auto width = line.width.has_value() ? *line.width : ;*/
		const auto width = line.width.value_or(defaultWidth);
		const auto pixelWidth = getQuadPixelSizeY(width);
		shapeRenderer.lineInstances.push_back(LineInstance{
			.transform = camera.makeTransform(line.start + vector / 2.0f, vector.angle(), Vec2(vector.length() / 2.0f + width, width)),
			.color = Vec4(line.color, 1.0f),
			.smoothing = 3.0f / pixelWidth,
			.lineWidth = width,
			.lineLength = vector.length() + width * 2.0f,
		});
	}

	Dbg::update();
}
