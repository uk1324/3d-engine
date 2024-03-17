#include <water_simulation/FunctionPlotter2d.hpp>
#include <water_simulation/Shaders/plotShaderData.hpp>
#include <game/Renderer.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Quat.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Math/Utils.hpp>
#include <imgui/imgui.h>
#include <engine/Utils/Timer.hpp>
#include <Dbg.hpp>

// This quad is not flat if the vertices are not coplanar.
void addQuad(std::vector<Vec3>& v, const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& v3) {
	v.push_back(v0);
	v.push_back(v1);
	v.push_back(v2);

	v.push_back(v0);
	v.push_back(v2);
	v.push_back(v3);
}

struct IndexedMesh {
	std::vector<Vec3> vertices;
	std::vector<u32> indices;
};

IndexedMesh makeIndexedMeshExact(const std::vector<Vec3>& trianglesVertices) {
	ASSERT(trianglesVertices.size() % 3 == 0);

	IndexedMesh mesh;
	std::unordered_map<Vec3, u32> map;
	for (int i = 0; i < trianglesVertices.size(); i++) {
		const auto& vertex = trianglesVertices[i];
		const auto result = map.insert({ vertex, static_cast<u32>(mesh.vertices.size()) });
		if (const auto newInserted = result.second) {
			mesh.vertices.push_back(vertex);
		}
		mesh.indices.push_back(result.first->second);
	}
	return mesh;
}

Vec3 triangleNormal(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
	return cross(v1 - v0, v2 - v0).normalized();
}

std::vector<Vec3> calculateFlatShadedNormals(const std::vector<Vec3>& trianglesVertices) {
	std::vector<Vec3> normals;
	ASSERT(trianglesVertices.size() % 3 == 0);
	for (int i = 0; i < trianglesVertices.size(); i += 3) {
		normals.push_back(triangleNormal(trianglesVertices[i], trianglesVertices[i + 1], trianglesVertices[i + 2]));
	}
	return normals;
}

std::vector<Vec3> calculateSmoothShededNormals(const IndexedMesh& mesh) {
	// @Performance not optmized. Could be done in a lot of different ways with different tradeoffs.
	std::unordered_multimap<u32, u32> vertexIndexToTriangleStartIndices;
	for (u32 i = 0; i < mesh.indices.size(); i += 3) {
		vertexIndexToTriangleStartIndices.insert({ mesh.indices[i], i });
		vertexIndexToTriangleStartIndices.insert({ mesh.indices[i + 1], i });
		vertexIndexToTriangleStartIndices.insert({ mesh.indices[i + 2], i });
	}

	std::vector<Vec3> normals;
	normals.reserve(mesh.vertices.size());
	for (u32 i = 0; i < mesh.vertices.size(); i++) {
		const auto range = vertexIndexToTriangleStartIndices.equal_range(i);
		Vec3 normal(0.0f);
		i32 trianglesSharingVertex = 0;
		float twiceTotalAreaOfTrianglesSharingVertex = 0.0f;
		for (auto it = range.first; it != range.second; ++it) {
			auto triangleStart = it->second;

			const auto v0 = mesh.vertices[mesh.indices[triangleStart]];
			const auto v1 = mesh.vertices[mesh.indices[triangleStart + 1]];
			const auto v2 = mesh.vertices[mesh.indices[triangleStart + 2]];
			const auto result = cross(v1 - v0, v2 - v0);
			//const auto length = result.length();
			normal += result;
			/*totalAreaOfTrianglesSharingVertex += ;
			trianglesSharingVertex++;*/
		}
		normal = normal.normalized();
		normals.push_back(normal);
		//normal /= 
		//ASSERT(normal != Vec3(0.0f));
	}

	//vertexIndexToTriangleStartIndices.
	/*ASSERT(trianglesVertices.size() % 3 == 0);
	for (int i = 0; i < trianglesVertices.size(); i += 3) {
		normals.push_back(cross(trianglesVertices[i + 1] - trianglesVertices[i], trianglesVertices[i + 2] - trianglesVertices[i]).normalized());
	}
	return normals;*/
	return normals;
}

u32 sphereIndicesSize = 0;

#include <iomanip>

std::vector<BasicShadingVertex> graphVertices;

const usize INFINTE_LINE_VBO_SIZE = 1024 * 1024;
const auto INSTANCE_BUFFER_SIZE = 1024ull * 10ull;

FunctionPlotter2d FunctionPlotter2d::make() {

	Vbo instancesVbo(INSTANCE_BUFFER_SIZE);
	//Vbo triangleVbo(triangle3dVertices, sizeof(triangle3dVertices));

	auto basicShadingVao = Vao::generate();

	basicShadingVao.bind();
	//PlotShaderShader::addAttributesToVao(basicShadingVao, triangleVbo, instancesVbo);


	Vbo infiniteLinesVbo(INFINTE_LINE_VBO_SIZE);
	auto infiniteLinesVao = Vao::generate();
	// TODO: make 2 function one for instances and one for vertex attributes.
	//InfiniteLineShader::addAttributesToVao(infiniteLinesVao, infiniteLinesVbo, instancesVbo);
	//{
	//	/*infiniteLinesVao.bind();
	//	infiniteLinesVao.bind();
	//	boundVaoSetAttribute(0, ShaderDataType::Float, 4, 0, sizeof(InfiniteLineVertex));
	//	glVertexAttribDivisor(0, 0);
	//	Vao::unbind();*/
	//}

	Vec2 v[]{
		{ -1.0, -1.0 },
		{ 1.0, -1.0 },
		{ 1.0, 1.0 },

		{ -1.0, -1.0 },
		{ 1.0, 1.0 },
		{ -1.0, 1.0 },
	};

	auto quadVao = Vao::generate();
	Vbo quadVbo(v, sizeof(v));
	quadVao.bind();
	quadVao.bind();
	boundVaoSetAttribute(0, ShaderDataType::Float, 2, 0, sizeof(Vec2));


	{
		std::vector<Vec3> graph2dVerticesPositions;
		//const auto STEPS = 500;
		const auto STEPS = 100;
		//const auto RANGE = 1000.0f;
		const auto RANGE = 4.0f;
		auto indexToValue = [&](i32 i) -> float {
			const auto a = (static_cast<float>(i) / STEPS - 0.5f) * RANGE;
			return a;
			};

		auto evaluate = [](float x, float y) -> float {
			return x * x * x - y * x;
		};

		auto position = [&](float x, float y) -> Vec3 {
			Vec3 p(x, evaluate(x, y), y);
			return p;
			//return Vec4(p, -1.0f).normalized().xyz();
			/*vec3 projected = normalize(vec4(p, -1)).xyz;
			return Vec3(x, evaluate(x, y), y);*/
		};

		Timer t;
		for (i32 xi = 0; xi < STEPS; xi++) {
			const auto x = indexToValue(xi);
			const auto nx = indexToValue(xi + 1);

			for (i32 yi = 0; yi < STEPS; yi++) {
				const auto y = indexToValue(yi);
				const auto ny = indexToValue(yi + 1);

				addQuad(graph2dVerticesPositions, position(x, y), position(nx, y), position(nx, ny), position(x, ny));
			}
		}
		dbg(t.elapsedMilliseconds());

		Timer t0;
		const auto normals = calculateFlatShadedNormals(graph2dVerticesPositions);
		for (int i = 0; i < normals.size(); i++) {
			for (int j = 0; j < 3; j++) {
				graphVertices.push_back(BasicShadingVertex{ .position = graph2dVerticesPositions[i * 3 + j], .normal = normals[i] });
			}
		}
		dbg(t0.elapsedMilliseconds());
	}
	auto graph2dVbo = Vbo(std::span<const BasicShadingVertex>(graphVertices));
	auto graph2dVao = Vao::generate();
	PlotShaderShader::addAttributesToVao(graph2dVao, graph2dVbo, instancesVbo);

	auto mainColorTexture = Texture::generate();
	auto mainDepthTexture = Texture::generate();
	auto mainFbo = Fbo::generate();

	GLint maxSamples;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

	Fbo::unbind();

#define MOVE(name) .name = std::move(name)

	return FunctionPlotter2d{
		.instancesVbo = std::move(instancesVbo),
		//.triangleVbo = std::move(triangleVbo),
		//.triangleVao = std::move(basicShadingVao),
	/*	MOVE(infiniteLinesVao),
		MOVE(infiniteLinesVbo),
		.infinteLinesShader = ShaderProgram::compile(INFINITE_LINE_SHADER_VERT_PATH, INFINITE_LINE_SHADER_FRAG_PATH),*/
		.basicShadingShader = ShaderProgram::compile(BASIC_SHADING_SHADER_VERT_PATH, BASIC_SHADING_SHADER_FRAG_PATH),
		.movementController = {
			.position = Vec3(3.0f, 2.0f, 0.0f),
			/*.movementSpeed = 5.0f*/
		},
		MOVE(quadVbo),
		MOVE(quadVao),
		MOVE(graph2dVbo),
		MOVE(graph2dVao),
	};
}

template<typename Instance, typename DrawFunction>
static void drawInstances(Vao& vao, Vbo& instancesVbo, const std::vector<Instance>& instances, DrawFunction drawFunction) {
	vao.bind();
	instancesVbo.bind();
	const auto maxInstancesPerDrawCall = INSTANCE_BUFFER_SIZE / sizeof(Instance);
	i32 drawn = 0;
	while (drawn < instances.size()) {
		const auto leftToDraw = instances.size() - drawn;
		const auto toDrawInThisDrawCall = (leftToDraw > maxInstancesPerDrawCall) ? maxInstancesPerDrawCall : leftToDraw;
		boundVboSetData(0, instances.data() + drawn, toDrawInThisDrawCall * sizeof(Instance));
		drawFunction(toDrawInThisDrawCall);
		drawn += toDrawInThisDrawCall;
	}
}

#include <engine/Graphics/Fbo.hpp>

void FunctionPlotter2d::update() {
	Window::disableCursor();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, Window::size().x, Window::size().y);
	glEnable(GL_DEPTH_TEST);

	const auto dt = 1.0f / 60.0f;
	elapsed += dt;
	if (!Window::isCursorEnabled()) {
		movementController.update(dt);
	} else {
		movementController.lastMousePosition = std::nullopt;
	}
	ImGui::InputFloat3("test", movementController.forward.data());
	const auto view = movementController.viewMatrix();
	const auto projection = Mat4::perspective(degToRad(90.0f), Window::aspectRatio(), 0.1f, 1000.0f);
	const auto viewProjection = view * projection;

	std::vector<BasicShadingInstance> graph2dInstances;
	basicShadingShader.use();
	graph2dInstances.push_back(BasicShadingInstance{ .transform = viewProjection });
	drawInstances(graph2dVao, instancesVbo, graph2dInstances, [](usize count) {
		glDrawArraysInstanced(GL_TRIANGLES, 0, graphVertices.size(), count);
	});
}
