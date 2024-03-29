#pragma once

#include <engine/Math/Vec3.hpp>
#include <engine/Math/Mat4.hpp>
#include <engine/Graphics/Vbo.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/Fbo.hpp>
#include <engine/Graphics/Texture.hpp>
#include <game/Shaders/basicShadingData.hpp>
#include <game/FpsController.hpp>
#include <array>

struct Vertex3d {
	Vec3 position;
};

//struct Camera3d {
//	Vec3 position;
//	float yRotation;
//	float xRotation;
//};

struct Renderer {
	static Renderer make();

	void update();

	Vec2 lastMousePosition;

	Vbo instancesVbo;
	Vbo triangleVbo;
	Vao triangleVao;

	Vbo infinitePlaneVbo;
	Vao infinitePlaneVao;
	Ibo infinitePlaneIbo;

	Vao infiniteLinesVao;
	Vbo infiniteLinesVbo;
	ShaderProgram infinteLinesShader;

	Vao sphereVao;
	Vbo sphereVbo;

	Vao sphereIndexedVao;
	Vbo sphereIndexedVbo;
	Ibo sphereIndexedIbo;

	ShaderProgram infinitePlaneShader;

	Mat4 transformTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2);

	ShaderProgram basicShadingShader;

	FpsController movementController;

	float elapsed = 0.0f;

	ShaderProgram shaderFrontInit;
	ShaderProgram shaderFrontPeel;
	ShaderProgram shaderFrontBlend;
	ShaderProgram shaderFrontFinal;
	Vbo quadVbo;
	Vao quadVao;

	struct DepthPeeling {
		static DepthPeeling make(Vec2 screenSize);

		std::array<Fbo, 2> fbos;
		std::array<Texture, 2> depthTextures;
		std::array<Texture, 2> colorTextures;
	} depthPeeling;

	Fbo mainFbo;
	Texture mainColorTexture;
	Texture mainDepthTexture;

	Vbo graph2dVbo;
	Vao graph2dVao;
};