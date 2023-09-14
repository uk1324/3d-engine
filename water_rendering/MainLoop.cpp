#include <water_rendering/MainLoop.hpp>
#include <water_rendering/ShaderManager.hpp>
#include <water_rendering/Shaders/waterShaderData.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Graphics/Texture.hpp>
#include <array>

namespace CubemapDirection {
	enum {
		POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		NEGATIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		COUNT = 6
	};
}

static const CubemapVertex cubeVertices[] = {
	Vec3(-1.0f, 1.0f, -1.0f), 
	Vec3(-1.0f, -1.0f, -1.0f), 
	Vec3(1.0f, -1.0f, -1.0f), 
	Vec3(1.0f, -1.0f, -1.0f), 
	Vec3(1.0f, 1.0f, -1.0f), 
	Vec3(-1.0f, 1.0f, -1.0f), 
	Vec3(-1.0f, -1.0f, 1.0f), 
	Vec3(-1.0f, -1.0f, -1.0f), 
	Vec3(-1.0f, 1.0f, -1.0f), 
	Vec3(-1.0f, 1.0f, -1.0f), 
	Vec3(-1.0f, 1.0f, 1.0f), 
	Vec3(-1.0f, -1.0f, 1.0f), 
	Vec3(1.0f, -1.0f, -1.0f), 
	Vec3(1.0f, -1.0f, 1.0f), 
	Vec3(1.0f, 1.0f, 1.0f), 
	Vec3(1.0f, 1.0f, 1.0f), 
	Vec3(1.0f, 1.0f, -1.0f), 
	Vec3(1.0f, -1.0f, -1.0f), 
	Vec3(-1.0f, -1.0f, 1.0f), 
	Vec3(-1.0f, 1.0f, 1.0f), 
	Vec3(1.0f, 1.0f, 1.0f), 
	Vec3(1.0f, 1.0f, 1.0f), 
	Vec3(1.0f, -1.0f, 1.0f), 
	Vec3(-1.0f, -1.0f, 1.0f), 
	Vec3(-1.0f, 1.0f, -1.0f), 
	Vec3(1.0f, 1.0f, -1.0f), 
	Vec3(1.0f, 1.0f, 1.0f), 
	Vec3(1.0f, 1.0f, 1.0f), 
	Vec3(-1.0f, 1.0f, 1.0f), 
	Vec3(-1.0f, 1.0f, -1.0f), 
	Vec3(-1.0f, -1.0f, -1.0f), 
	Vec3(-1.0f, -1.0f, 1.0f), 
	Vec3(1.0f, -1.0f, -1.0f), 
	Vec3(1.0f, -1.0f, -1.0f), 
	Vec3(-1.0f, -1.0f, 1.0f), 
	Vec3(1.0f, -1.0f, 1.0f)
	/*Vec3(-1.0f,-1.0f,-1.0f),
	Vec3(-1.0f,-1.0f, 1.0f),
	Vec3(-1.0f, 1.0f, 1.0f),
	Vec3(1.0f, 1.0f,-1.0f),
	Vec3(-1.0f,-1.0f,-1.0f),
	Vec3(-1.0f, 1.0f,-1.0f),
	Vec3(1.0f,-1.0f, 1.0f),
	Vec3(-1.0f,-1.0f,-1.0f),
	Vec3(1.0f,-1.0f,-1.0f),
	Vec3(1.0f, 1.0f,-1.0f),
	Vec3(1.0f,-1.0f,-1.0f),
	Vec3(-1.0f,-1.0f,-1.0f),
	Vec3(-1.0f,-1.0f,-1.0f),
	Vec3(-1.0f, 1.0f, 1.0f),
	Vec3(-1.0f, 1.0f,-1.0f),
	Vec3(1.0f,-1.0f, 1.0f),
	Vec3(-1.0f,-1.0f, 1.0f),
	Vec3(-1.0f,-1.0f,-1.0f),
	Vec3(-1.0f, 1.0f, 1.0f),
	Vec3(-1.0f,-1.0f, 1.0f),
	Vec3(1.0f,-1.0f, 1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(1.0f,-1.0f,-1.0f),
	Vec3(1.0f, 1.0f,-1.0f),
	Vec3(1.0f,-1.0f,-1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(1.0f,-1.0f, 1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(1.0f, 1.0f,-1.0f),
	Vec3(-1.0f, 1.0f,-1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(-1.0f, 1.0f,-1.0f),
	Vec3(-1.0f, 1.0f, 1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(-1.0f, 1.0f, 1.0f),
	Vec3(1.0f,-1.0f, 1.0),*/
};

Texture loadCubemap(const std::array<const char*, CubemapDirection::COUNT>& paths) {
	auto cubemap = Texture::generate();
	cubemap.bind(GL_TEXTURE_CUBE_MAP);

	for (i32 i = 0; i < CubemapDirection::COUNT; i++) {
		Image32 image(paths[i]);
		glTexImage2D(CubemapDirection::POSITIVE_X + i, 0, GL_RGB, image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, image.data());
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return cubemap;
}

template<>
struct std::hash<WaterShaderVertex> {
	std::size_t operator()(const WaterShaderVertex& value) const noexcept {
		return std::hash<Vec2>()(value.position);
	}
};

template<typename Vertex>
std::pair<std::vector<u32>, std::vector<Vertex>> makeIndexedMeshExact(const std::vector<Vertex>& trianglesVertices) {
	ASSERT(trianglesVertices.size() % 3 == 0);

	std::vector<Vertex> vertices;
	std::vector<u32> indices;
	std::unordered_map<Vertex, u32> map;
	for (int i = 0; i < trianglesVertices.size(); i++) {
		const auto& vertex = trianglesVertices[i];
		const auto result = map.insert({ vertex, static_cast<u32>(vertices.size()) });
		if (const auto newInserted = result.second) {
			vertices.push_back(vertex);
		}
		indices.push_back(result.first->second);
	}
	return { indices, vertices };
}

MainLoop MainLoop::make() {
	Vbo instancesVbo(1024ull * 10);

	std::vector<WaterShaderVertex> waterVertices;
	const auto COUNT = 500;
	for (i32 ix = 0; ix < COUNT; ix++) {
		for (i32 iy = 0; iy < COUNT; iy++) {
			const auto SIZE = 100.0f;
			auto convert = [&](Vec2 v) -> WaterShaderVertex {
				return WaterShaderVertex{ (v / COUNT) * SIZE };
			};
			const auto v0 = convert(Vec2(ix, iy));
			const auto v1 = convert(Vec2(ix + 1, iy));
			const auto v2 = convert(Vec2(ix + 1, iy + 1));
			const auto v3 = convert(Vec2(ix, iy + 1));
			waterVertices.push_back(v0);
			waterVertices.push_back(v1);
			waterVertices.push_back(v2);

			waterVertices.push_back(v0);
			waterVertices.push_back(v2);
			waterVertices.push_back(v3);
		}
	}
	//usize waterVertexCount = static_cast<usize>(COUNT) * COUNT * 6;
	const auto [indices, vertices] = makeIndexedMeshExact(waterVertices);

	auto waterVbo = Vbo(std::span<const WaterShaderVertex>(vertices));
	auto waterIbo = Ibo(indices.data(), indices.size() * sizeof(u32));
	auto waterVao = Vao::generate();
	WaterShaderInstances::addAttributesToVao(waterVao, waterVbo, instancesVbo);
	waterVao.bind();
	waterIbo.bind();
	Vao::unbind();

	auto cubemapVao = Vao::generate();
	auto cubemapVbo = Vbo(cubeVertices, sizeof(cubeVertices));
	CubemapShaderInstances::addAttributesToVao(cubemapVao, cubemapVbo, instancesVbo);

	Window::disableCursor();
#define MOVE(name) .name = std::move(name)
	return MainLoop{
		MOVE(instancesVbo),

		MOVE(waterVao),
		MOVE(waterVbo),
		MOVE(waterIbo),
		.waterIndexCount = indices.size(),
		.waterShader = MAKE_GENERATED_SHADER(WATER_SHADER),

		MOVE(cubemapVao),
		MOVE(cubemapVbo),
		.cubemapVertexCount = std::size(cubeVertices),
		.cubemapShader = MAKE_GENERATED_SHADER(CUBEMAP_SHADER)
	};
}

void MainLoop::update() {
	const auto dt = 1.0f / 6.0f;
	elapsed += dt;

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
		return;
	}
	if (Input::isKeyDown(KeyCode::P)) {
		Window::toggleCursor();
	}

	if (!Window::isCursorEnabled()) {
		movementController.update(dt);
	} else {
		movementController.lastMousePosition = std::nullopt;
	}

	if (Input::isKeyDown(KeyCode::V)) {
		GLint polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);

		if (polygonMode == GL_FILL) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	ShaderManager::update();
	
	glViewport(0, 0, Window::size().x, Window::size().y);
	const auto skyColor = Vec3(178, 255, 255) / 255.0f;
	glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	const auto view = movementController.viewMatrix();
	const auto projection = Mat4::perspective(90.0f, Window::aspectRatio(), 0.1f, 100.0f);
	const auto viewProjection = view * projection;

	{
		glDepthMask(GL_FALSE);
		shaderSetUniforms(cubemapShader, CubemapShaderVertUniforms{
			.transform = view.removedTranslation() * projection
		});
		cubemapShader.use();
		cubemapVao.bind(); 
		cubemapShaderFragUniforms.update();
		glDrawArrays(GL_TRIANGLES, 0, cubemapVertexCount);

		glDepthMask(GL_TRUE);
	}

	{
		std::vector<WaterShaderInstance> waterInstances;
		waterInstances.push_back(WaterShaderInstance{
			.transform = viewProjection
		});
		waterShader.use();
		shaderSetUniforms(waterShader, WaterShaderVertUniforms{
			.time = elapsed
		});
		shaderSetUniforms(waterShader, WaterShaderFragUniforms{
			.cameraPosition = movementController.position
		});
		drawInstances(waterVao, instancesVbo, waterInstances, [&](usize count) {
			//glDrawArraysInstanced(GL_TRIANGLES, 0, waterI, count);
			glDrawElements(GL_TRIANGLES, waterIndexCount, GL_UNSIGNED_INT, nullptr);
		});
	}
}
