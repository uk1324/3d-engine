#include <game/Renderer.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Quat.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Math/Utils.hpp>
#include <game/Shaders/infinitePlaneData.hpp>
#include <game/Shaders/infiniteLineData.hpp>
#include <imgui/imgui.h>
#include <map>
#include <queue>
#include <Dbg.hpp>

#include "AdaptiveSample.hpp"

GLuint g_frontFboId[2];
GLuint g_frontDepthTexId[2];
GLuint g_frontColorTexId[2];
GLuint g_frontColorBlenderTexId;
GLuint g_frontColorBlenderFboId;

#define GL_TEXTURE_RECTANGLE_ARB GL_TEXTURE_RECTANGLE
#define GL_DEPTH_COMPONENT32F_NV GL_DEPTH_COMPONENT32F
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER
#define GL_DEPTH_ATTACHMENT_EXT GL_DEPTH_ATTACHMENT
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0
#define glGenFramebuffersEXT glGenFramebuffers

GLenum g_drawBuffers[] = { 
	GL_COLOR_ATTACHMENT0_EXT,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6
};

#define glBindFramebufferEXT glBindFramebuffer
#define glFramebufferTexture2DEXT glFramebufferTexture2D

void InitFrontPeelingRenderTargets()
{
	auto g_imageWidth = Window::size().x, g_imageHeight = Window::size().y;


	glGenTextures(2, g_frontDepthTexId);
	glGenTextures(2, g_frontColorTexId);
	glGenFramebuffers(2, g_frontFboId);

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, g_frontDepthTexId[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT32F_NV,
			g_imageWidth, g_imageHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, g_frontColorTexId[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, g_imageWidth, g_imageHeight,
			0, GL_RGBA, GL_FLOAT, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_frontFboId[i]);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			GL_TEXTURE_RECTANGLE_ARB, g_frontDepthTexId[i], 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_RECTANGLE_ARB, g_frontColorTexId[i], 0);
	}

	glGenTextures(1, &g_frontColorBlenderTexId);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, g_frontColorBlenderTexId);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, g_imageWidth, g_imageHeight,
		0, GL_RGBA, GL_FLOAT, 0);

	glGenFramebuffersEXT(1, &g_frontColorBlenderFboId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_frontColorBlenderFboId);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
		GL_TEXTURE_RECTANGLE_ARB, g_frontDepthTexId[0], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
		GL_TEXTURE_RECTANGLE_ARB, g_frontColorBlenderTexId, 0);
	//CHECK_GL_ERRORS;
}

const auto g_numPasses = 3;
const auto g_useOQ = false;

ShaderProgram* g_shaderFrontInit;
ShaderProgram* g_shaderFrontPeel;
ShaderProgram* g_shaderFrontBlend;
ShaderProgram* g_shaderFrontFinal;

void RenderFrontToBackPeeling(auto DrawModel)
{
	// ---------------------------------------------------------------------
	// 1. Initialize Min Depth Buffer
	// ---------------------------------------------------------------------

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_frontColorBlenderFboId);
	glDrawBuffer(g_drawBuffers[0]);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	/*g_shaderFrontInit->bind();*/
	g_shaderFrontInit->use();
	//g_shaderFrontInit.setUniform("Alpha", (float*)&g_opacity, 1);
	DrawModel();
	//g_shaderFrontInit.unbind();
	//g_shaderFrontInit.unbind();
	//ShaderProgram::

	//CHECK_GL_ERRORS;

	// ---------------------------------------------------------------------
	// 2. Depth Peeling + Blending
	// ---------------------------------------------------------------------

	int numLayers = (g_numPasses - 1) * 2;
	for (int layer = 1; g_useOQ || layer < numLayers; layer++) {
		int currId = layer % 2;
		int prevId = 1 - currId;

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_frontFboId[currId]);
		glDrawBuffer(g_drawBuffers[0]);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		/*if (g_useOQ) {
			glBeginQuery(GL_SAMPLES_PASSED_ARB, g_queryId);
		}*/
		/*if (g_useOQ) {
			glBeginQuery(GL_SAMPLES_PASSED, g_queryId);
		}*/

		//g_shaderFrontPeel.bind();
		//g_shaderFrontPeel.bindTextureRECT("DepthTex", g_frontDepthTexId[prevId], 0);
		////g_shaderFrontPeel.setUniform("Alpha", (float*)&g_opacity, 1);
		//DrawModel();
		//g_shaderFrontPeel.unbind();

		auto setTextureUnit = [](std::string texname, int texunit, int _progId)
		{
			GLint linked;
			glGetProgramiv(_progId, GL_LINK_STATUS, &linked);
			if (linked != GL_TRUE) {
				std::cerr << "Error: setTextureUnit needs program to be linked." << std::endl;
				exit(1);
			}
			GLint id = glGetUniformLocation(_progId, texname.c_str());
			if (id == -1) {
		#ifdef NV_REPORT_UNIFORM_ERRORS
				std::cerr << "Warning: Invalid texture " << texname << std::endl;
		#endif
				return;
			}
			glUniform1i(id, texunit);
		};

		g_shaderFrontPeel->use();
		auto bindTexture = [&](GLenum target, std::string texname, GLuint texid, int texunit, int prog)
		{
			glActiveTexture(GL_TEXTURE0 + texunit);
			glBindTexture(target, texid);
			setTextureUnit(texname, texunit, prog);
			glActiveTexture(GL_TEXTURE0);
		};
		auto bindTextureRECT = [&](std::string texname, GLuint texid, int texunit, int prog) {
			bindTexture(GL_TEXTURE_RECTANGLE_ARB, texname, texid, texunit, prog);
		};

		bindTextureRECT("DepthTex", g_frontDepthTexId[prevId], 0, g_shaderFrontPeel->handle());
		//g_shaderFrontPeel->bindTextureRECT("DepthTex", g_frontDepthTexId[prevId], 0);

		//bindTexture(GL_TEXTURE_RECTANGLE_ARB, texname, texid, texunit);
		//g_shaderFrontPeel.setUniform("Alpha", (float*)&g_opacity, 1);
		DrawModel();
		//g_shaderFrontPeel.unbind();


		if (g_useOQ) {
			//glEndQuery(GL_SAMPLES_PASSED_ARB);
		}

		//CHECK_GL_ERRORS;

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_frontColorBlenderFboId);
		glDrawBuffer(g_drawBuffers[0]);

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE,
			GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

		/*g_shaderFrontBlend.bind();
		g_shaderFrontBlend.bindTextureRECT("TempTex", g_frontColorTexId[currId], 0);
		glCallList(g_quadDisplayList);
		g_shaderFrontBlend.unbind();*/
		g_shaderFrontBlend->use();
		bindTextureRECT("TempTex", g_frontColorTexId[currId], 0, g_shaderFrontBlend->handle());
		/*g_shaderFrontBlend.bindTextureRECT("TempTex", g_frontColorTexId[currId], 0);
		glCallList(g_quadDisplayList); ?????????????????????
		g_shaderFrontBlend.unbind();*/

		glDisable(GL_BLEND);

		//CHECK_GL_ERRORS;

		/*if (g_useOQ) {
			GLuint sample_count;
			glGetQueryObjectuiv(g_queryId, GL_QUERY_RESULT_ARB, &sample_count);
			if (sample_count == 0) {
				break;
			}
		}*/
	}

	// ---------------------------------------------------------------------
	// 3. Final Pass
	// ---------------------------------------------------------------------

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDrawBuffer(GL_BACK);
	glDisable(GL_DEPTH_TEST);

	g_shaderFrontFinal->use();
	/*g_shaderFrontFinal.bind();*/
	//g_shaderFrontFinal.setUniform("BackgroundColor", g_backgroundColor, 3);
	//g_shaderFrontFinal.bindTextureRECT("ColorTex", g_frontColorBlenderTexId, 0);
	//glCallList(g_quadDisplayList); ??????????????????????????????
	//g_shaderFrontFinal.unbind();

	//CHECK_GL_ERRORS;
}

const auto INSTANCE_BUFFER_SIZE = 1024ull * 10ull;

BasicShadingVertex triangle3dVertices[]{
	{ Vec3(1.0f, 0.0f, 1.0f) },
	{ Vec3(0.0f, 1.0f, 1.0f) },
	{ Vec3(0.0f, 0.0f, 1.0f) }
};

// Uses homogeneous coordinates
// The only difference between trasforming a normal point and a point at infinity is that points at infinity don't get translated, because the w basis in the matrix gets multiplied by zero.
// When using a perspective matrix the w coorindate of the output vector is indepentent of the w coorindate of the input vector. The projection matrix just copies the the z value. So setting w = 0 doesn't cause division by zero.
InfinitePlaneVertex vertices[]{
	// I don't think it's possible to draw an infinite plane using only 2 triangles. I think that might be because the vertices don't get translated so the same thing is rendered no matter the camera position. If you actually try it you will see something like a flickering 1D checkboard. This might be because the view direction is always parallel with the plane.
	{ Vec4(0, 0, 0, 1) },
	{ Vec4(1, 0, 0, 0) },
	{ Vec4(0, 0, 1, 0) },
	{ Vec4(-1, 0, 0, 0) },
	{ Vec4(0, 0, -1, 0) },
};

i32 infinitePlaneIndices[]{
	0, 1, 2,
	0, 2, 3,
	0, 3, 4,
	0, 4, 1
};

// For drawing points could draw a plane that has a constant depth written to the depth buffer.
// Procedural skybox texture.
// Movement on a topological model of the projective plane. Somehow draw curves on it.
// Make a graph from where you can move to where.
// Could use subdivision on the mesh.
// Color each face differently also use depth peeling.
// Maybe it might be better to use the parametric representation.
// Construct a topological model of the projective plane by using a parabolic point.

const usize INFINTE_LINE_VBO_SIZE = 1024 * 1024;

std::vector<Vec3> sphereVertices;

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

IndexedMesh makeIndexedMesh(const std::vector<Vec3>& trianglesVertices, float epsilon) {
	ASSERT(trianglesVertices.size() % 3 == 0);

	IndexedMesh mesh;
	// TODO: Either do a O(n^2) approach or make some acceleration structure like a kd tree.
	// https://softwareengineering.stackexchange.com/questions/311173/how-to-find-the-closest-vector-to-a-given-vector
	// In general you shuldn't use a map with an epslion. One thing that might work would be rounding the value to the nearest multiple of epslion i guess.
	//std::priority_queue<std::pair<Vec3, u32>> queue;

	//struct Comarator {
	//	bool operator()(const Vec3& a, const Vec3& b) {
	//		return a.distanceSquaredTo(b);
	//	}
	//};

	//std::vector<std::pair<Vec3, u32>> map;

	//for (int i = 0; i < trianglesVertices.size(); i++) {
	//	const auto& vertex = trianglesVertices[i];
	//	if (std::lower_bound(map.begin(), map.end(), vertex, ))
	//	/*auto result = map.push_back({ vertex, static_cast<u32>(mesh.vertices.size()) });
	//	if (const auto newInserted = result.second) {
	//		mesh.vertices.push_back(vertex);
	//	}
	//	mesh.indices.push_back(result.first->second);*/
	//}
	return mesh;
}

Vec3 triangleNormal(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
	return cross(v1 - v0, v2 - v0).normalized();
}

float triangleArea(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
	// It is probably slower, but could try projecting the triangle onto a plane (using dot product I think) (constructing a plane already requires calculating a normal or normalizing a vector so this would be slower) and then doing a cross product.
	return cross(v1 - v0, v2 - v0).length() / 2.0f;
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

Renderer Renderer::make() {
	InitFrontPeelingRenderTargets();

	Vbo instancesVbo(INSTANCE_BUFFER_SIZE);
	Vbo triangleVbo(triangle3dVertices, sizeof(triangle3dVertices));

	auto basicShadingVao = Vao::generate();

	basicShadingVao.bind();
	BasicShadingInstances::addAttributesToVao(basicShadingVao, triangleVbo, instancesVbo);

	Vbo infinitePlaneVbo(vertices, sizeof(vertices));
	auto infinitePlaneVao = Vao::generate();
	InfinitePlaneInstances::addAttributesToVao(infinitePlaneVao, infinitePlaneVbo, instancesVbo);
	Ibo infinitePlaneIbo(infinitePlaneIndices, sizeof(infinitePlaneIndices));

	infinitePlaneVao.bind();
	infinitePlaneIbo.bind();
	Vao::unbind();
	Ibo::unbind();

	Vbo infiniteLinesVbo(INFINTE_LINE_VBO_SIZE);
	auto infiniteLinesVao = Vao::generate();
	// TODO: make 2 function one for instances and one for vertex attributes.
	InfiniteLineInstances::addAttributesToVao(infiniteLinesVao, infiniteLinesVbo, instancesVbo);
	//{
	//	/*infiniteLinesVao.bind();
	//	infiniteLinesVao.bind();
	//	boundVaoSetAttribute(0, ShaderDataType::Float, 4, 0, sizeof(InfiniteLineVertex));
	//	glVertexAttribDivisor(0, 0);
	//	Vao::unbind();*/
	//}

	const auto STEPS = 17;
	for (int ix = 0; ix < STEPS; ix++) {
		for (int iy = 0; iy < STEPS; iy++) {			
			// Would it be better to subtract 0.5f after division or before somehow?
			float x = (ix / static_cast<float>(STEPS) - 0.5f) * 2.0f;
			float y = (iy / static_cast<float>(STEPS) - 0.5f) * 2.0f;
			// Doing this istead of adding step to x to prevent precision issues.
			float nextX = ((ix + 1) / static_cast<float>(STEPS) - 0.5f) * 2.0f;
			float nextY = ((iy + 1) / static_cast<float>(STEPS) - 0.5f) * 2.0f;

			Vec2 v0(x, y);
			Vec2 v1(nextX, y);
			Vec2 v2(nextX, nextY);
			Vec2 v3(x, nextY);

			auto projectQuadsOntoSphere = [&](auto swizzleFunction, auto flipFunction) {
				const auto a0 = swizzleFunction(v0).normalized();
				const auto a1 = swizzleFunction(v1).normalized();
				const auto a2 = swizzleFunction(v2).normalized();
				const auto a3 = swizzleFunction(v3).normalized();

				addQuad(sphereVertices, a0, a1, a2, a3);
				// Can't just negate the whole vector, because then neighbouring vertices don't have the exact same values because of precision issues.
				addQuad(sphereVertices, flipFunction(a3), flipFunction(a2), flipFunction(a1), flipFunction(a0));
			};

			projectQuadsOntoSphere([](Vec2 v) { return Vec3(1.0f, v.x, v.y); }, [](Vec3 v) { return Vec3(-v.x, v.y, v.z); });
			projectQuadsOntoSphere([](Vec2 v) { return Vec3(v.x, -1.0f, v.y); }, [](Vec3 v) { return Vec3(v.x, -v.y, v.z); });
			projectQuadsOntoSphere([](Vec2 v) { return Vec3(v.x, v.y, 1.0f); }, [](Vec3 v) { return Vec3(v.x, v.y, -v.z); });
		}
	}

	std::vector<BasicShadingVertex> sphereBasicShadingVertices;
	const auto sphereFlatNormals = calculateFlatShadedNormals(sphereVertices);
	for (int i = 0; i < sphereFlatNormals.size(); i++) {
		sphereBasicShadingVertices.push_back(BasicShadingVertex{ .position = sphereVertices[i * 3], .normal = sphereFlatNormals[i] });
		sphereBasicShadingVertices.push_back(BasicShadingVertex{ .position = sphereVertices[i * 3 + 1], .normal = sphereFlatNormals[i] });
		sphereBasicShadingVertices.push_back(BasicShadingVertex{ .position = sphereVertices[i * 3 + 2], .normal = sphereFlatNormals[i] });
	}
 
	Vbo sphereVbo(sphereBasicShadingVertices.data(), sphereBasicShadingVertices.size() * sizeof(decltype(sphereBasicShadingVertices)::value_type));
	auto sphereVao = Vao::generate();
	BasicShadingInstances::addAttributesToVao(sphereVao, sphereVbo, instancesVbo);
	//InfiniteLineInstances::addAttributesToVao(infiniteLinesVao, infiniteLinesVbo, instancesVbo);

	IndexedMesh mesh = makeIndexedMeshExact(sphereVertices);
	sphereIndicesSize = mesh.indices.size();
	const auto indexedSphereNormals = calculateSmoothShededNormals(mesh);

	std::vector<BasicShadingVertex> indexedSphereVertices;
	for (i32 i = 0; i < mesh.vertices.size(); i++) {
		indexedSphereVertices.push_back(BasicShadingVertex{ .position = mesh.vertices[i], .normal = indexedSphereNormals[i]});
		/*const auto c = Color::fromHsv(float(i) / mesh.vertices.size(), 1.0f, 1.0f).xyz();
		indexedSphereVertices.push_back(BasicShadingVertex{ .position = mesh.vertices[i], .normal = c });*/
	}

	//Vbo sphereIndexedVbo(std::span<const Vec3>(mesh.vertices));
	Vbo sphereIndexedVbo(indexedSphereVertices.data(), indexedSphereVertices.size() * sizeof(decltype(indexedSphereVertices)::value_type));
	/*Vbo sphereIndexedVbo(sphereBasicShadingVertices.data(), sphereBasicShadingVertices.size() * sizeof(decltype(sphereBasicShadingVertices)::value_type));;*/
	auto sphereIndexedVao = Vao::generate();
	/*BasicShadingInstances::addAttributesToVao(sphereIndexedVao, sphereIndexedVbo, instancesVbo);*/
	BasicShadingInstances::addAttributesToVao(sphereIndexedVao, sphereIndexedVbo, instancesVbo);
	/*BasicShadingInstances::addAttributesToVao(sphereIndexedVao, sphereIndexedVbo, instancesVbo);*/
	Ibo sphereIndexedIbo(mesh.indices.data(), mesh.indices.size() * sizeof(u32));

	sphereIndexedVao.bind();
	sphereIndexedIbo.bind();
	Vao::unbind();
	Ibo::unbind();


#define MOVE(name) .name = std::move(name)

	return Renderer{
		.lastMousePosition = Vec2(0.0f),
		.instancesVbo = std::move(instancesVbo),
		.triangleVbo = std::move(triangleVbo),
		.triangleVao = std::move(basicShadingVao),
		.infinitePlaneVbo = std::move(infinitePlaneVbo),
		.infinitePlaneVao = std::move(infinitePlaneVao),
		.infinitePlaneIbo = std::move(infinitePlaneIbo),
		MOVE(infiniteLinesVao),
		MOVE(infiniteLinesVbo),
		.infinteLinesShader = ShaderProgram::compile(INFINITE_LINE_SHADER_VERT_PATH, INFINITE_LINE_SHADER_FRAG_PATH),
		MOVE(sphereVao),
		MOVE(sphereVbo),
		MOVE(sphereIndexedVao),
		MOVE(sphereIndexedVbo),
		MOVE(sphereIndexedIbo),
		.infinitePlaneShader = ShaderProgram::compile(INFINITE_PLANE_SHADER_VERT_PATH, INFINITE_PLANE_SHADER_FRAG_PATH),
		.basicShadingShader = ShaderProgram::compile(BASIC_SHADING_SHADER_VERT_PATH, BASIC_SHADING_SHADER_FRAG_PATH),
		.movementController = {
			.position = Vec3(3.0f, 2.0f, 0.0f),
			.movementSpeed = 5.0f
		},
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

void Renderer::update() {
	Fbo::unbind();

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
	infloat(farZ, 20000.0f);
	auto target = movementController.position + movementController.cameraForwardRotation() * Vec3::FORWARD;
	const auto view = Mat4::lookAt(movementController.position, target, Vec3::UP);
	const auto projection = Mat4::perspective(degToRad(90.0f), Window::aspectRatio(), 0.1f, farZ);
	const auto viewProjection = view * projection;

	ImGui::InputFloat3("test", movementController.position.data());
	Gui::put("look dir %", (target - movementController.position).roundedToDecimalDigits(2));


	std::vector<InfinitePlaneInstance> instances;

	infloat(rotationX, 0.0f);
	infloat(rotationY, 0.0f);
	infloat(translationY, 0.0f);
	//rotation += 0.01f;
	const auto model = Mat4(Mat3::rotationX(rotationX) * Mat3::rotationY(rotationY)) * Mat4::translation(Vec3(0.0f, translationY, 0.0f));

	instances.push_back(InfinitePlaneInstance{
		.transform = model * viewProjection,
	});
	infinitePlaneShader.use();

	InfinitePlaneFragUniforms uniforms{
		.inverseViewProjection = viewProjection.inversed(),
		.screenSize = Window::size(),
	};
	shaderSetUniforms(infinitePlaneShader, uniforms);
	drawInstances(infinitePlaneVao, instancesVbo, instances, [](usize instances) { 
		glDrawElementsInstanced(GL_TRIANGLES, std::size(infinitePlaneIndices), GL_UNSIGNED_INT, 0, instances);
	});

	BasicShadingInstances a;

	instances.clear();

	
	/*a.toDraw.push_back(BasicShadingInstance{
		.transform = Mat4::translation(Vec3(0.0f, 0.0f, 1.0f)) * viewProjection
	});*/
	basicShadingShader.use();
	drawInstances(triangleVao, instancesVbo, a.toDraw, [](usize instances) {
		glDrawArraysInstanced(GL_TRIANGLES, 0, std::size(triangle3dVertices), instances);
	});

	a.toDraw.clear();

	std::vector<InfiniteLineVertex> vertices;

	auto drawRay = [&vertices](Vec3 start, Vec3 direction, Vec3 color) {
		vertices.push_back(InfiniteLineVertex{ Vec4(start, 1.0f), color });
		vertices.push_back(InfiniteLineVertex{ Vec4(direction, 0.0f), color });
	};

	auto drawLine = [&vertices](Vec3 start, Vec3 end, Vec3 color) {
		vertices.push_back(InfiniteLineVertex{ Vec4(start, 1.0f), color });
		vertices.push_back(InfiniteLineVertex{ Vec4(end, 1.0f), color });
	};
	
	//const auto range = 2.0f;
	// function grapher jit compiler

	/*insliderfloat(range, 80.0f, 1.0f, 80.0f);*/
	//insliderfloat(val, 0.0f, -2.0f, 2.0f);
	//auto sample = [&](float x) {
	//	/*return ((sin(2.0f * x) + 1.0f) / 2.0f + 0.01f) * x * x;*/
	//	/*return ((sin(5.0f * x) + 1.0f) / 2.0f + 1.0f) * x * x;*/
	//	/*return ((cos(2.0f * x) + 1.0f) / 2.0f + 1.0f) * x * x;*/
	//	//return pow(x, 6.0f);
	//	/*return pow(x, 2.0f);*/
	//	/*const auto a = sqrt(-(pow(x, 2.0f) * (1.0f - val) - 2.0f * x * val + 1));
	//	ASSERT(isnormal(a));*/
	//	return sqrt(-(pow(x, 2.0f) * (1.0f - val) - 2.0f * x * val + 1));
	//	//return abs(x);
	//};

	auto projectOntoHemisphere = [](Vec2 pos) -> Vec3 {
		// project a point from plane y = -1 to sphere x^2 + y^2 + z^2 = 1
		return Vec3(pos.x, -1.0f, pos.y).normalized();
	};


	function_sampler_1d_options options;
	function_sampler_1d_options_defaults(&options);
	auto sampler = function_sampler_1d_new(&options);

	std::vector<float> discontinouties;
	chk(disable);
	insliderfloat(val, 0.0f, -2.0f, 2.0f);

	{
		float a = -1.0f + pow(val, 2.0f);
		float b = -2.0f * val;
		float c = 1.0f - 0.00001f;

		if (a == 0.0f) {
			discontinouties.push_back(-c / b);
		} else {
			const auto discriminant = pow(b, 2.0f) - 4.0f * a * c;
			if (discriminant == 0.0f) {
				discontinouties.push_back(-b / (2.0f * a));
			} else if (discriminant > 0.0f) {
				const auto s = sqrt(discriminant);
				discontinouties.push_back((-b + s) / (2.0f * a));
				discontinouties.push_back((-b - s) / (2.0f * a));
			}
		}

		
	}

	auto plot = [&](auto sample) {
		function_sampler_1d_clear(sampler);
		int count = 750;
		const auto range = 120.0f;

		auto calculateX = [&](int i) -> float {
			auto t = ((static_cast<float>(i) / count) - 0.5f) * range;
			return t;
		};

		for (int i = 0; i <= count; i++) {
			auto x = calculateX(i);
			function_sampler_1d_add(sampler, x, sample(x), 0);
		}

		for (auto a : discontinouties) {
			function_sampler_1d_add(sampler, a, sample(a), 0);
		}
		function_sampler_1d_add(sampler, -2000.0f, sample(-2000.0f), 0);
		function_sampler_1d_add(sampler, 2000.0f, sample(2000.0f), 0);

		int loops = 0;

		while (!function_sampler_1d_is_done(sampler) && loops <= 2) {
			loops++;

			const auto num = function_sampler_1d_num_refine(sampler);
			std::vector<double> toRefine;
			toRefine.resize(num);
			const auto nret = function_sampler_1d_get_refine(sampler, num, toRefine.data());

			for (int i = 0; i < nret; i++) {
				dbgGui(toRefine[i]);
				if (!disable)
					function_sampler_1d_add(sampler, toRefine[i], sample(toRefine[i]), 0);
			}
		}

		const auto numSamples = function_sampler_1d_num_samples(sampler);
		for (int i = 0; i < numSamples - 1; i++) {
			double x, y;
			int _;
			function_sampler_1d_get(sampler, i, &x, &y, &_);
			Vec2 current(x, y);
			function_sampler_1d_get(sampler, i + 1, &x, &y, &_);
			Vec2 next(x, y);

			auto to3d = [](Vec2 p) -> Vec3 {
				return Vec3(p.x, 0.1f, p.y);
			};

			drawLine(to3d(current), to3d(next), Color3::GREEN);
			const auto a = projectOntoHemisphere(current) * 1.01f;
			const auto b = projectOntoHemisphere(next) * 1.01f;
			drawLine(a + Vec3::UP, b + Vec3::UP, Color3::RED);
			drawLine(-a + Vec3::UP, -b + Vec3::UP, Color3::RED);
		}
	};
	
	/*for (const auto& d : discontinouties) {
		drawLine(Vec3(d, 0.1f, -2.0f), Vec3(d, 0.1f, 2.0f), Color3::WHITE);
	}*/

	function_sampler_1d_clear(sampler);


	/*struct QuadraticSolutions {

	};*/

	auto conicSections = [](float x) {
		/*return sqrt(-(pow(x, 2.0f) * (1.0f - val) - 2.0f * x * val - 1));*/
		//return sqrt(-(pow(x, 2.0f) * (1.0f - val) - 2.0f * x * val - 1));
		return sqrt(-pow(x, 2.0f) * (1.0f - pow(val, 2.0f)) - 2.0f * val * x + 1.0f);
	};

	/*for (const auto& d : discontinouties) {
		drawLine(Vec3(d, 0.1f, -2.0f), Vec3(d, 0.1f, 2.0f), Color3::WHITE);
	}*/

	auto conicSectionsPlot = [&](float x) {
		const auto v = conicSections(x);
		return v;
	};

	plot([&](float x) {
		return conicSectionsPlot(x);
	});

	plot([&](float x) {
		return -conicSectionsPlot(x);
	});

	auto plotParametric = [&](auto sampleParametric, float range, Vec3 groundColor, Vec3 sphereColor) {
		const auto count = 800;
		auto calculateT = [&count, &range](int i) -> float {
			return ((static_cast<float>(i) / count) - 0.5f) * range;
		};
		
		Vec2 previous = sampleParametric(calculateT(0));
		for (int i = 1; i <= count; i++) {
			const auto t = ((static_cast<float>(i) / count) - 0.5f) * range;
			const auto current = sampleParametric(t);
			auto to3d = [](Vec2 p) -> Vec3 {
				return Vec3(p.x, 0.1f, p.y);
			};

			drawLine(to3d(current), to3d(previous), groundColor);
			const auto a = projectOntoHemisphere(previous) * 1.01f;
			const auto b = projectOntoHemisphere(current) * 1.01f;
			drawLine(a + Vec3::UP, b + Vec3::UP, sphereColor);
			drawLine(-a + Vec3::UP, -b + Vec3::UP, sphereColor);

			previous = current;
		}
	};


	insliderfloat(c, 0.0f, 0.0f, 1.0f);
	infloat(speed, 0.001f);
	static float direction = 1.0f;
	chk(animate) {
		if (c > 1.0f || c < 0.0f) {
			direction = -direction;
		}
		c += speed * direction;
	}





	insliderfloat(testa, 10.0f, 10.0f, 50.0f);













	//auto plot = [&](auto sample) {
	//	int count = 2600;
	//	const auto range = 120.0f;

	//	auto calculateX = [&](int i) -> float {
	//		auto t = ((static_cast<float>(i) / count) - 0.5f) * range;
	//		/*return pow(t / 5.0f, 3.0f);*/
	//		return pow(t / testa, 3.0f);
	//		//return t;
	//	};

	//	Vec3 previous = Vec3(calculateX(0), 0.1f, sample(calculateX(0)));
	//	for (int i = 1; i <= count; i++) {
	//		//auto x = ((static_cast<float>(i) / count) - 0.5f) * range;
	//		auto x = calculateX(i);
	//		Vec3 current = Vec3(x, 0.1f, sample(x));

	//		drawLine(previous, current, Color3::GREEN);
	//		const auto a = projectOntoHemisphere(Vec2(previous.x, previous.z)) * 1.01f;
	//		const auto b = projectOntoHemisphere(Vec2(current.x, current.z)) * 1.01f;
	//		drawLine(a + Vec3::UP, b + Vec3::UP, Color3::RED);
	//		drawLine(-a + Vec3::UP, -b + Vec3::UP, Color3::RED);
	//		
	//		if (isnan(previous.z) && !isnan(current.z)) {
	//			int x = 5;
	//		}

	//		previous = current;
	//	}
	//};

	//insliderfloat(val, 0.0f, -2.0f, 2.0f);

	//std::vector<float> discontinouties;

	//{
	//	float a = -1.0f + pow(val, 2.0f);
	//	float b = -2.0f * val;
	//	float c = 1.0f;

	//	const auto discriminant = pow(b, 2.0f) - 4.0f * a * c;
	//	if (discriminant == 0.0f) {
	//		discontinouties.push_back(-b / (2.0f * a));
	//	} else if (discriminant > 0.0f) {
	//		const auto s = sqrt(discriminant);
	//		discontinouties.push_back((-b + s) / (2.0f * a));
	//		discontinouties.push_back((-b - s) / (2.0f * a));
	//	}
	//}


	///*struct QuadraticSolutions {

	//};*/

	//auto conicSections = [](float x) {
	//	/*return sqrt(-(pow(x, 2.0f) * (1.0f - val) - 2.0f * x * val - 1));*/
	//	//return sqrt(-(pow(x, 2.0f) * (1.0f - val) - 2.0f * x * val - 1));
	//	return sqrt(-pow(x, 2.0f) * (1.0f - pow(val, 2.0f)) - 2.0f * val * x + 1.0f);
	//};

	///*for (const auto& d : discontinouties) {
	//	drawLine(Vec3(d, 0.1f, -2.0f), Vec3(d, 0.1f, 2.0f), Color3::WHITE);
	//}*/

	//auto conicSectionsPlot = [&](float x) {
	//	const auto v = conicSections(x);

	//	if (isnan(v)) {
	//		for (const auto& d : discontinouties) {
	//			if (abs(d - x) < 0.04615384615f / 2.0f) {
	//				return 0.0f;
	//				return conicSections(d);
	//			}
	//		}
	//	}
	//	return v;
	//};



	//plot([&](float x) {
	//	return conicSectionsPlot(x);
	//});

	//plot([&](float x) {
	//	return -conicSectionsPlot(x);
	//});

	//auto plotParametric = [&](auto sampleParametric, float range, Vec3 groundColor, Vec3 sphereColor) {
	//	const auto count = 800;
	//	auto calculateT = [&count, &range](int i) -> float {
	//		return ((static_cast<float>(i) / count) - 0.5f) * range;
	//	};
	//	
	//	Vec2 previous = sampleParametric(calculateT(0));
	//	for (int i = 1; i <= count; i++) {
	//		const auto t = ((static_cast<float>(i) / count) - 0.5f) * range;
	//		const auto current = sampleParametric(t);
	//		auto to3d = [](Vec2 p) -> Vec3 {
	//			return Vec3(p.x, 0.1f, p.y);
	//		};

	//		drawLine(to3d(current), to3d(previous), groundColor);
	//		const auto a = projectOntoHemisphere(previous) * 1.01f;
	//		const auto b = projectOntoHemisphere(current) * 1.01f;
	//		drawLine(a + Vec3::UP, b + Vec3::UP, sphereColor);
	//		drawLine(-a + Vec3::UP, -b + Vec3::UP, sphereColor);

	//		previous = current;
	//	}
	//};

	//function_sampler_1d_options options;
	//function_sampler_1d_options_defaults(&options);
	//auto sampler = function_sampler_1d_new(&options);

	///*plot([&](float t) -> Vec2 {
	//	return Vec2(cosh(t), sinh(t));
	//}, 40.0f);
	//plot([&](float t) -> Vec2 {
	//	return Vec2(-cosh(t), sinh(t));
	//}, 40.0f);*/


	//insliderfloat(c, 0.0f, 0.0f, 1.0f);
	//infloat(speed, 0.001f);
	//static float direction = 1.0f;
	//chk(animate) {
	//	if (c > 1.0f || c < 0.0f) {
	//		direction = -direction;
	//	}
	//	c += speed * direction;
	//}











	/*plot([&](float t) -> Vec2 {
		return lerp(Vec2(sin(t), cos(t)), Vec2(cosh(t), sinh(t)), c);
	}, 80.0f, Color3::RED, Color3::GREEN);


	plot([&](float t) -> Vec2 {
		return lerp(Vec2(sin(t), cos(t)), Vec2(-cosh(t), -sinh(t)), c);
	}, 80.0f, Color3::RED, Color3::GREEN);*/
	/*infloat(offset, 0.0f);
	plot([&](float t) -> Vec2 {
		return Vec2(sin(t), cos(t) + offset);
	}, 80.0f, Color3::RED, Color3::GREEN);*/

	std::vector<BasicShadingInstance> sphereInstances;
	//sphereInstances.push_back(BasicShadingInstance{
	//	/*.transform = Mat4(Mat3::scale(2.0f)) * Mat4::translation(Vec3(0.0f, 2.0f, 0.0f)) * viewProjection*/
	//	.transform = Mat4::translation(Vec3(0.0f, 1.0f, 0.0f)) * viewProjection
	//});

	basicShadingShader.use();
	drawInstances(sphereIndexedVao, instancesVbo, sphereInstances, [](usize count) {
		glDrawElementsInstanced(GL_TRIANGLES, sphereIndicesSize, GL_UNSIGNED_INT, nullptr, count);
	});

	sphereInstances.clear();

	{
		infinteLinesShader.use();
		shaderSetUniforms(infinteLinesShader, InfiniteLineVertUniforms{ .viewProjection = viewProjection });

 		glDepthFunc(GL_LEQUAL);
		const auto verticesSize = sizeof(InfiniteLineVertex) * vertices.size();
		if (verticesSize > INFINTE_LINE_VBO_SIZE) {
			ASSERT_NOT_REACHED();
		}
		infiniteLinesVbo.setData(0, vertices.data(), verticesSize);
		infiniteLinesVao.bind();
		glDrawArrays(GL_LINES, 0, vertices.size());

		glDepthFunc(GL_LESS);
	}
	infiniteLinesVbo.bind();
	
}

Mat4 Renderer::transformTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
	const Mat4 translateBasis{
		v1 - v0,
		v2 - v0,
		Vec4(0.0f),
		Vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
	return translateBasis * Mat4::translation(v0);
}

