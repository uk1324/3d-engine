#include <water_rendering/MainLoop.hpp>
#include <water_rendering/ShaderManager.hpp>
#include <water_rendering/Shaders/debugPointShaderData.hpp>
#include <water_rendering/Shaders/heightMapShaderData.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Graphics/Texture.hpp>
#include <engine/Math/Frustum.hpp>
#include <engine/Math/Color.hpp>
#include <Timer.hpp>
#include <array>

int GRID_SIZE = 300;

struct Grid
{

	std::vector<std::vector<float>> mesh;

	Grid(float init)
	{
		std::vector<float> i_grid(GRID_SIZE, 0);
		mesh = std::vector<std::vector<float>>(GRID_SIZE, i_grid);

		for (int ii = 0; ii < GRID_SIZE; ii++)
		{
			for (int jj = 0; jj < GRID_SIZE; jj++)
			{

				mesh[ii][jj] = init;
			}
		}
	}
};
Grid water_h(1.0f); // water hight
Grid vel_u(0.0f);	// water velocity in x direction
Grid vel_v(0.0f);	// water velocity in y dirction

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

const auto WATER_TILE_SIZE = 100.0f;

MainLoop MainLoop::make() {
	Vbo instancesVbo(1024ull * 10);

	std::vector<WaterShaderVertex> waterVertices;
	const auto COUNT = 100;
	for (i32 ix = 0; ix < COUNT; ix++) {
		for (i32 iy = 0; iy < COUNT; iy++) {
			auto convert = [&](Vec2 v) -> WaterShaderVertex {
				return WaterShaderVertex{ ((v / COUNT) - Vec2(0.5f)) * WATER_TILE_SIZE };
				//return WaterShaderVertex{ (v / COUNT) * WATER_TILE_SIZE };
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

	auto heightMapVao = Vao::generate();
	auto heightMapVbo = Vbo(GRID_SIZE * GRID_SIZE * 6ull * sizeof(HeightMapVertex));
	HeightMapShaderInstances::addAttributesToVao(heightMapVao, heightMapVbo, instancesVbo);
	auto heightMap = Texture::generate();
	heightMap.bind();
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_R32F,
		GRID_SIZE,
		GRID_SIZE,
		0,
		GL_RED,
		GL_FLOAT,
		nullptr
	);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glGenerateMipmap(GL_TEXTURE_2D);

	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(settings.wrapS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(settings.wrapT));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(settings.minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(settings.magFilter));*/


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
	auto ret = MainLoop{
		.movementController = {
			.position = Vec3(48.0f, 32.0f, 48.0f)
		},
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

		MOVE(heightMapVao),
		MOVE(heightMapVbo),
		.heightMapShader = MAKE_GENERATED_SHADER(HEIGHT_MAP_SHADER),
		MOVE(heightMap),

		MOVE(debugPointVao),
		MOVE(debugPointVbo),
		.debugPointShader = MAKE_GENERATED_SHADER(DEBUG_POINT_SHADER),

		.ubo = ubo
	};
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			ret.height[i][j] = 3.0f;
			ret.vel[i][j] = Vec2(0.0f);
		}
	}

	for (int ii = 20; ii < 30; ii++)
	{
		for (int jj = 20; jj < 30; jj++)
		{
			water_h.mesh[ii][jj] = 3.0f;
		}
	}
	return ret;
}

void MainLoop::update() {
	ImGui::Checkbox("paused", &paused);
	auto dt = 1.0f / 6.0f;
	if (!paused) {
		elapsed += dt;
	}

	Timer timer;

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


	auto sample = [&](i64 x, i64 y) -> float {
		return height[std::clamp(x - 1, 0ll, GRID_SIZE - 1)][std::clamp(y - 1, 0ll, GRID_SIZE - 1)];
	};

	auto heightMapToMesh = [](float* height, i64 size) -> std::vector<HeightMapVertex> {
		std::vector<HeightMapVertex> v;
		for (i32 ix = 0; ix < size - 1; ix++) {
			for (i32 iy = 0; iy < size - 1; iy++) {
				auto convert = [&](i64 x, i64 y) -> HeightMapVertex {
					//return HeightMapVertex{ Vec3(static_cast<float>(x), height[x * size + y], static_cast<float>(y)) };
					return HeightMapVertex{ Vec3(static_cast<float>(x), 0.0f, static_cast<float>(y)) };
				};
				auto v0 = convert(ix, iy);
				auto v1 = convert(ix + 1, iy);
				auto v2 = convert(ix + 1, iy + 1);
				auto v3 = convert(ix, iy + 1);

				float dhdx = v1.position.x - v0.position.x;
				float dhdy = v3.position.y - v0.position.y;
				const auto normal = (cross(Vec3(1.0f, dhdx, 0.0f), Vec3(0.0f, dhdy, 1.0f))).normalized();
				v0.normal = normal;
				v1.normal = normal;
				v2.normal = normal;
				v3.normal = normal;

				v.push_back(v0);
				v.push_back(v2);
				v.push_back(v1);

				v.push_back(v0);
				v.push_back(v3);
				v.push_back(v2);
			}
		}
		return v;
	};

	float physicsDt = 1.0f / 60.0f;
	//float physicsDt = 1.0f;

	float g = 9.81f;
	//float dt = 0.1f;
	int GRID_SIZE = 50;
	float dxdy = 0.4f;
	float pix_step = 5.0f;

	if (Input::isKeyDown(KeyCode::H)) {
		water_h.mesh[5][5] = 128.0f;
		/*for (int ii = 5; ii < 10; ii++) {
			for (int jj = 5; jj < 10; jj++) {
				height[ii][jj] += 128.0f;
			}
		}*/
	}


	//ImGui::Begin("Simulation", nullptr, window_flags);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();


	for (int ii = 1; ii < GRID_SIZE - 1; ii++)
	{

		for (int jj = 1; jj < GRID_SIZE - 1; jj++)
		{

			float dh_dx = [=]()
			{
				float f = (water_h.mesh[ii + 1][jj] - water_h.mesh[ii - 1][jj]) / (2 * dxdy);

				return f;
			}();

			float dh_dy = [=]()
			{
				float f = (water_h.mesh[ii][jj + 1] - water_h.mesh[ii][jj - 1]) / (2 * dxdy);

				return f;
			}();

			vel_u.mesh[ii][jj] = vel_u.mesh[ii][jj] - dt * g * dh_dx;
			vel_v.mesh[ii][jj] = vel_v.mesh[ii][jj] - dt * g * dh_dy;

			float du_dx = [=]()
			{
				float f = (vel_u.mesh[ii + 1][jj] - vel_u.mesh[ii - 1][jj]) / (2 * dxdy);

				return f;
			}();

			float dv_dy = [=]()
			{
				float f = (vel_v.mesh[ii][jj + 1] - vel_v.mesh[ii][jj - 1]) / (2 * dxdy);

				return f;
			}();

			water_h.mesh[ii][jj] = water_h.mesh[ii][jj] + dt * (-(du_dx + dv_dy));
		}
	}

	for (int ii = 0; ii < GRID_SIZE; ii++)
	{
		for (int jj = 0; jj < GRID_SIZE; jj++)
		{
			float D = 0.0;
			ImVec2 p0;
			ImVec2 p1;
			p0 = { (float)ii * pix_step, (float)jj * pix_step };
			p1 = { ((float)ii * pix_step) + pix_step, ((float)jj * pix_step) + pix_step };

			float hi = water_h.mesh[ii][jj] * 50.0f;

			ImVec4 pix = ImVec4(0.0f, 0.0f, hi - 0.3f, 1.0f);

			draw_list->AddRectFilled(p0, p1, ImColor(pix));

		}
	}


	//for (int ii = 1; ii < GRID_SIZE - 1; ii++)
	//{

	//	for (int jj = 1; jj < GRID_SIZE - 1; jj++)
	//	{

	//		float dh_dx = [=]()
	//		{
	//			//float f = (height[ii + 1][jj] - height[ii - 1][jj]) / (2 * dxdy);
	//			float f = (height[ii + 1][jj] - height[ii - 1][jj]) / (2 * dxdy);

	//			return f;
	//		}();

	//		float dh_dy = [=]()
	//		{
	//			float f = (height[ii][jj + 1] - height[ii][jj - 1]) / (2 * dxdy);

	//			return f;
	//		}();

	//		vel[ii][jj].x = vel[ii][jj].x - physicsDt * g * dh_dx;
	//		vel[ii][jj].y = vel[ii][jj].y - physicsDt * g * dh_dy;

	//		float du_dx = [=]()
	//		{
	//			float f = (vel[ii + 1][jj].x - vel[ii - 1][jj].x) / (2 * dxdy);

	//			return f;
	//		}();

	//		float dv_dy = [=]()
	//		{
	//			float f = (vel[ii][jj + 1].y - vel[ii][jj - 1].y) / (2 * dxdy);

	//			return f;
	//		}();

	//		height[ii][jj] = height[ii][jj] + physicsDt * (-(du_dx + dv_dy));
	//	}
	//}

	/*if (Input::isKeyDown(KeyCode::H)) {
		height[0][0] = 1.0;
	}
	for (i64 ix = 0; ix < GRID_SIZE; ix++) {
		for (i64 iy = 0; iy < GRID_SIZE; iy++) {
			float dhdx = (sample(ix - 1, iy) - sample(ix + 1, iy)) / 2.0f;
			float dhdy = (sample(ix, iy - 1) - sample(ix, iy + 1)) / 2.0f;
			dhdt[ix][iy] = -(dhdx + dhdy);
		}
	}

	for (i64 ix = 0; ix < GRID_SIZE; ix++) {
		for (i64 iy = 0; iy < GRID_SIZE; iy++) {
			height[ix][iy] += physicsDt * dhdt[ix][iy];
		}
	}*/

	heightMap.bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GRID_SIZE, GRID_SIZE, GL_RED, GL_FLOAT, height);
	auto mesh = heightMapToMesh(reinterpret_cast<float*>(height), GRID_SIZE);
	heightMapVbo.bind();
	/*GLint size;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);*/
	boundVboSetData(0, mesh.data(), mesh.size() * sizeof(HeightMapVertex));

	heightMapShader.use();
	std::vector<HeightMapShaderInstance> inst;
	inst.push_back(HeightMapShaderInstance{ .transform = viewProjection });
	
	glActiveTexture(GL_TEXTURE0);
	heightMap.bind();
	glActiveTexture(GL_TEXTURE0);
	heightMapShader.setTexture("heightMap", 0);

	drawInstances(heightMapVao, instancesVbo, inst, [&](usize count) {
		glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.size(), count);
	});
	//ImGui::Image(reinterpret_cast<void*>(heightMap.handle()), ImVec2(1.0f, 1.0f));
 //	for (i64 ix = 0; ix < GRID_SIZE; ix++) {
	//	for (i64 iy = 0; iy < GRID_SIZE; iy++) {
	//		auto convert = [](i64 v) {
	//			return static_cast<float>(v);
	//		};
	//		const auto x = convert(ix);
	//		const auto y = convert(iy);
	//		addPoint(Vec3(x, height[ix][iy], y));
	//		/*addPoint(Vec3(x, height[ix][iy], y));*/
	//	}
	//}
	//{
	//	std::vector<WaterShaderInstance> waterInstances;
	//	const auto count = 14;
	//	//const auto count = 8;
	//	int rendered = 0;
	//	/*waterInstances.push_back(WaterShaderInstance{
	//		.transform = viewProjection,
	//		.offset = WATER_TILE_SIZE * Vec2(0)
	//	});*/
	//	for (i32 xi = -count; xi < count; xi++) {
	//		for (i32 yi = -count; yi < count; yi++) {
	//			const auto center = Vec2(xi, yi) * WATER_TILE_SIZE;
	//			const auto min = center - Vec2(WATER_TILE_SIZE / 2.0f);
	//			const auto max = center + Vec2(WATER_TILE_SIZE / 2.0f);
	//			const auto aabb = Aabb3::fromMinMax(Vec3(min.x, -19.0f, min.y), Vec3(max.x, 10.0f, max.y));
	//			const auto corners = aabb.corners();
	//			/*for (auto& corner : corners) {
	//				addPoint(corner);
	//			}*/

	//			if (!frustum.intersects(aabb)) {
	//				continue;
	//			}
	//			rendered++;

	//			waterInstances.push_back(WaterShaderInstance{
	//				.transform = viewProjection,
	//				.offset = WATER_TILE_SIZE * Vec2(xi, yi)
	//			});
	//		}
	//	}
	//	Gui::put("rendered: %\ntotal: %", rendered, pow(count * 2.0f, 2.0f));
	//	/*waterInstances.push_back(WaterShaderInstance{
	//		.transform = viewProjection,
	//		.offset = Vec2(0.0f, 0.0f)
	//	});*/

	//	shaderSetUbo(waterShader, "SkyboxSettings", 0, ubo);

	//	waterShader.use();

	//	waterShaderVertUniforms.update();
	//	waterShaderVertUniforms.time = elapsed;
	//	shaderSetUniforms(waterShader, waterShaderVertUniforms);

	//	waterShaderFragUniforms.update();
	//	waterShaderFragUniforms.cameraPosition = movementController.position;
	//	waterShaderFragUniforms.directionalLightDirection = directionalLightDirection;

 //		shaderSetUniforms(waterShader, waterShaderFragUniforms);
	//	glEnable(GL_CULL_FACE);
	//	glCullFace(GL_FRONT);
	//	drawInstances(waterVao, instancesVbo, waterInstances, [&](usize count) {
	//		//glDrawArraysInstanced(GL_TRIANGLES, 0, waterI, count);
	//		glDrawElementsInstanced(GL_TRIANGLES, waterIndexCount, GL_UNSIGNED_INT, nullptr, count);
	//	});
	//}
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
	/*for (auto& p : frustumPoints) {
		addPoint(p);
	}*/

	{
		debugPointShader.use();
		drawInstances(debugPointVao, instancesVbo, points, [&](usize count) {
			glPointSize(10.0f);
			glDrawArraysInstanced(GL_POINTS, 0, 1, count);
		});
	}

	for (int ii = 0; ii < GRID_SIZE; ii++)
	{
		for (int jj = 0; jj < GRID_SIZE; jj++)
		{
			float D = 0.0;
			ImVec2 p0;
			ImVec2 p1;
			p0 = { (float)ii * pix_step, (float)jj * pix_step };
			p1 = { ((float)ii * pix_step) + pix_step, ((float)jj * pix_step) + pix_step };

			float hi = height[ii][jj] / 10.0f;

			ImVec4 pix = ImVec4(0.0f, 0.0f, hi - 0.3f, 1.0f);

			ImGui::GetWindowDrawList()->AddRectFilled(p0, p1, ImColor(pix));

		}
	}

	Gui::put("elapsed: %", timer.elapsedMilliseconds());
}
