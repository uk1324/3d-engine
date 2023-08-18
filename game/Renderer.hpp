#pragma once

#include <engine/Math/Vec3.hpp>
#include <engine/Math/Mat4.hpp>
#include <engine/Graphics/Vbo.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <game/Shaders/basicShadingData.hpp>
#include <game/FpsController.hpp>

struct Vertex3d {
	Vec3 position;
};

struct Camera3d {
	Vec3 position;
	float yRotation;
	float xRotation;
};

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

	ShaderProgram infinitePlaneShader;

	Mat4 transformTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2);

	ShaderProgram basicShadingShader;

	FpsController movementController;

	float elapsed = 0.0f;
};