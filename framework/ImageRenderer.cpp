#include <framework/ImageRenderer.hpp>
#include <framework/Instancing.hpp>
#include <StructUtils.hpp>
#include <glad/glad.h>

static constexpr TexturedQuadVertex fullscreenQuadVerts[]{
	{ Vec2{ -1.0f, 1.0f }, Vec2{ 0.0f, 1.0f } },
	{ Vec2{ 1.0f, 1.0f }, Vec2{ 1.0f, 1.0f } },
	{ Vec2{ -1.0f, -1.0f }, Vec2{ 0.0f, 0.0f } },
	{ Vec2{ 1.0f, -1.0f }, Vec2{ 1.0f, 0.0f } },
};

static constexpr u32 fullscreenQuadIndices[]{
	0, 1, 2, 2, 1, 3
};

ImageRenderer ImageRenderer::make() {
	auto instancesVbo = Vbo(1024ull);

	auto texturedQuadVbo = Vbo(fullscreenQuadVerts, sizeof(fullscreenQuadVerts));
	auto texturedQuadIbo = Ibo(fullscreenQuadIndices, sizeof(fullscreenQuadIndices));
	auto texturedQuadVao = Vao::generate();
	TexturedQuadInstances::addAttributesToVao(texturedQuadVao, texturedQuadVbo, instancesVbo);
	texturedQuadVao.bind();
	texturedQuadIbo.bind();
	Vao::unbind();
	Ibo::unbind();

	auto value = ImageRenderer{
		MOVE(instancesVbo),
		.texturedQuadShader = MAKE_GENERATED_SHADER(TEXTURED_QUAD),
		MOVE(texturedQuadVbo),
		MOVE(texturedQuadIbo),
		MOVE(texturedQuadVao),
	};
	return value;
}

void ImageRenderer::update() {

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
			glDrawElementsInstanced(GL_TRIANGLES, std::size(fullscreenQuadIndices), GL_UNSIGNED_INT, nullptr, instanceCount);
		});
		instances.clear();
	}

	for (auto& texture : textures) {
		texture.drawThisFrame = false;
	}
}

void ImageRenderer::drawImage(Pixel32* data, Vec2T<i64> size, const Mat3x2& transform) {
	TextureData* textureToSaveTo = nullptr;

	for (auto& texture : textures) {
		if (texture.size == size) {
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
			size.x,
			size.y,
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
			.size = size,
		});
		textureToSaveTo = &textures.back();
	}
	
	textureToSaveTo->texture.bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, FORMAT, TYPE, data);
	textureToSaveTo->drawThisFrame = true;
	textureToSaveTo->transform = transform;
}
