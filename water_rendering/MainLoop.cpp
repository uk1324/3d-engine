#include <water_rendering/MainLoop.hpp>
#include <water_rendering/ShaderManager.hpp>
#include <water_rendering/Shaders/debugPointShaderData.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Graphics/Texture.hpp>
#include <engine/Math/Frustum.hpp>
#include <engine/Math/Color.hpp>
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

const auto WATER_TILE_SIZE = 25.0f;

MainLoop MainLoop::make() {
	Vbo instancesVbo(1024ull * 10);

	std::vector<WaterShaderVertex> waterVertices;
	const auto COUNT = 200;
	for (i32 ix = 0; ix < COUNT; ix++) {
		for (i32 iy = 0; iy < COUNT; iy++) {
			auto convert = [&](Vec2 v) -> WaterShaderVertex {
				/*return WaterShaderVertex{ ((v / COUNT) - Vec2(0.5f)) * WATER_TILE_SIZE };*/
				return WaterShaderVertex{ (v / COUNT) * WATER_TILE_SIZE };
			};
			const auto v0 = convert(Vec2(ix, iy));
			const auto v1 = convert(Vec2(ix + 1, iy));
			const auto v2 = convert(Vec2(ix + 1, iy + 1));
			const auto v3 = convert(Vec2(ix, iy + 1));
			waterVertices.push_back(v0);
			waterVertices.push_back(v2);
			waterVertices.push_back(v1);

			waterVertices.push_back(v0);
			waterVertices.push_back(v3);
			waterVertices.push_back(v2);
		}
	}
	//usize waterVertexCount = static_cast<usize>(COUNT) * COUNT * 6;
	//const auto [indices, vertices] = makeIndexedMeshExact(waterVertices);
	std::vector<u32> indices;
	for (int i = 0; i < waterVertices.size(); i++) {
		indices.push_back(i);
	}
	auto& vertices = waterVertices;

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

	Vec3 point(0.0f);
	auto debugPointVao = Vao::generate();
	auto debugPointVbo = Vbo(point.data(), sizeof(point));
	DebugPointShaderInstances::addAttributesToVao(debugPointVao, debugPointVbo, instancesVbo);

	u32 ubo;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(SkyboxSettings), NULL, GL_DYNAMIC_DRAW);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SkyboxSettings) + 8, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

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
		.cubemapShader = MAKE_GENERATED_SHADER(CUBEMAP_SHADER),

		MOVE(debugPointVao),
		MOVE(debugPointVbo),
		.debugPointShader = MAKE_GENERATED_SHADER(DEBUG_POINT_SHADER),

		.ubo = ubo
	};
}

void MainLoop::update() {
	ImGui::Checkbox("paused", &paused);
	auto dt = 1.0f / 6.0f;
	if (!paused) {
		elapsed += dt;
	}

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
		return;
	}
	if (Input::isKeyDown(KeyCode::P)) {
		Window::toggleCursor();
	}

	if (!Window::isCursorEnabled()) {
		movementController.update(dt);

		if (Input::isKeyDown(KeyCode::V)) {
			GLint polygonMode;
			glGetIntegerv(GL_POLYGON_MODE, &polygonMode);

			if (polygonMode == GL_FILL) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			} else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
	} else {
		movementController.lastMousePosition = std::nullopt;
	}
	ImGui::InputFloat3("test", movementController.position.data());

	ShaderManager::update();
	
	glViewport(0, 0, Window::size().x, Window::size().y);
	const auto skyColor = Vec3(178, 255, 255) / 255.0f;
	glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	const auto view = movementController.viewMatrix();
	const auto projection = Mat4::perspective(90.0f, Window::aspectRatio(), 0.1f, 1000.0f);
	const auto viewProjection = view * projection;
	const auto directionalLightDirection = Vec3(1, -1, 0).normalized();
	const auto frustum = Frustum::fromMatrix(viewProjection);

	std::vector<DebugPointShaderInstance> points;
	auto addPoint = [&](Vec3 p) {
		points.push_back(DebugPointShaderInstance{
			.transform = Mat4::translation(p) * viewProjection,
			.color = Color3::RED,
		});
	};
	skyboxSettings.update();

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SkyboxSettings), static_cast<SkyboxSettings*>(&skyboxSettings));

	auto shaderSetUbo = [](ShaderProgram& shader, const char* uniformBlockName, u32 uniformBlockIndex, u32 ubo) {
		unsigned int index = glGetUniformBlockIndex(shader.handle(), uniformBlockName);
		ASSERT(index != GL_INVALID_INDEX);

		glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockIndex, ubo);
		glUniformBlockBinding(shader.handle(), index, uniformBlockIndex);
	};

	
	{
		shaderSetUbo(cubemapShader, "SkyboxSettings", 0, ubo);

		shaderSetUniforms(cubemapShader, CubemapShaderVertUniforms{
			.transform = view.removedTranslation() * projection,
		});
		cubemapShaderFragUniforms.directionalLightDirection = directionalLightDirection;
		cubemapShaderFragUniforms.time = elapsed;
		shaderSetUniforms(cubemapShader, cubemapShaderFragUniforms);
		cubemapShader.use();
		cubemapVao.bind(); 
		cubemapShaderFragUniforms.update();

		glDepthMask(GL_FALSE);
		glDrawArrays(GL_TRIANGLES, 0, cubemapVertexCount);
		glDepthMask(GL_TRUE);
	}
	{
		// @Performance: Frustum culling?
		std::vector<WaterShaderInstance> waterInstances;
		const auto count = 7;
		//const auto count = 8;
		int rendered = 0;
		/*waterInstances.push_back(WaterShaderInstance{
			.transform = viewProjection,
			.offset = WATER_TILE_SIZE * Vec2(0)
		});*/
		for (i32 xi = -count; xi < count; xi++) {
			for (i32 yi = -count; yi < count; yi++) {
				const auto center = Vec2(xi, yi) * WATER_TILE_SIZE;
				const auto min = center - Vec2(WATER_TILE_SIZE / 2.0f);
				const auto max = center + Vec2(WATER_TILE_SIZE / 2.0f);
				const auto aabb = Aabb3::fromMinMax(Vec3(min.x, -19.0f, min.y), Vec3(max.x, 10.0f, max.y));
				const auto corners = aabb.corners();
				/*for (auto& corner : corners) {
					addPoint(corner);
				}*/

				if (!frustum.intersects(aabb)) {
					continue;
				}
				rendered++;

				waterInstances.push_back(WaterShaderInstance{
					.transform = viewProjection,
					.offset = WATER_TILE_SIZE * Vec2(xi, yi)
				});
			}
		}
		Gui::put("rendered: %\ntotal: %", rendered, pow(count * 2.0f, 2.0f));
		/*waterInstances.push_back(WaterShaderInstance{
			.transform = viewProjection,
			.offset = Vec2(0.0f, 0.0f)
		});*/

		shaderSetUbo(waterShader, "SkyboxSettings", 0, ubo);

		waterShader.use();

		waterShaderVertUniforms.update();
		waterShaderVertUniforms.time = elapsed;
		shaderSetUniforms(waterShader, waterShaderVertUniforms);

		waterShaderFragUniforms.update();
		waterShaderFragUniforms.cameraPosition = movementController.position;
		waterShaderFragUniforms.directionalLightDirection = directionalLightDirection;

 		shaderSetUniforms(waterShader, waterShaderFragUniforms);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		drawInstances(waterVao, instancesVbo, waterInstances, [&](usize count) {
			//glDrawArraysInstanced(GL_TRIANGLES, 0, waterI, count);
			glDrawElementsInstanced(GL_TRIANGLES, waterIndexCount, GL_UNSIGNED_INT, nullptr, count);
		});
	}
	static i32 val;
	GUI_PROPERTY_EDITOR(Gui::inputI32("test", val));

	static std::vector<Vec3> frustumPoints;
	if (Input::isKeyDown(KeyCode::H)) {
		frustumPoints.clear();

		//const auto corners = Frustum::corners(viewProjection);
		const auto corners = Frustum::corners(view * Mat4::perspective(90.0f, Window::aspectRatio(), 0.1f, 20.0f));
		for (auto& corner : corners) {
			frustumPoints.push_back(corner);
		}
	}
	for (auto& p : frustumPoints) {
		addPoint(p);
	}

	{
		debugPointShader.use();
		drawInstances(debugPointVao, instancesVbo, points, [&](usize count) {
			glPointSize(10.0f);
			glDrawArraysInstanced(GL_POINTS, 0, 1, count);
		});
	}
}
