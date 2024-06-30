#pragma once

#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <engine/Math/Mat3x2.hpp>
#include <framework/Shaders/diskShaderData.hpp>
#include <framework/Shaders/circleShaderData.hpp>
#include <framework/Shaders/lineShaderData.hpp>
#include <framework/Shaders/filledAabbData.hpp>

struct ShapeRenderer2d {
	static ShapeRenderer2d make(Vbo& fullscreenQuad2dPtVerticesVbo, Ibo& fullscreenQuad2dPtVerticesIbo, Vbo& instancesVbo);

	void update(Vbo& instancesVbo);

	Vao diskVao;
	ShaderProgram& diskShader;
	std::vector<DiskInstance> diskInstances;

	Vao circleVao;
	ShaderProgram& circleShader;
	std::vector<CircleInstance> circleInstances;

	Vao lineVao;
	ShaderProgram& lineShader;
	std::vector<LineInstance> lineInstances;

	Vao filledAabbVao;
	ShaderProgram& filledAabbShader;
	std::vector<FilledAabbInstance> filledAabbInstances;

	Vao filledTriangleVao;
	ShaderProgram& filledTriangleShader;
};