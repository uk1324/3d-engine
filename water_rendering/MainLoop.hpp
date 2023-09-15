#pragma once
#include <game/FpsController.hpp>
#include <water_rendering/Instancing.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <water_rendering/SavableUniforms.hpp>
#include <water_rendering/Shaders/cubemapShaderData.hpp>

struct MainLoop {
	static MainLoop make();
	void update();

	float elapsed = 0.0f;

	FpsController movementController;

	Vbo instancesVbo;

	Vao waterVao;
	Vbo waterVbo;
	Ibo waterIbo;
	usize waterIndexCount;
	ShaderProgram& waterShader;

	Vao cubemapVao;
	Vbo cubemapVbo;
	const usize cubemapVertexCount;
	SavableUniforms<CubemapShaderFragUniforms> cubemapShaderFragUniforms = SavableUniforms<CubemapShaderFragUniforms>("water_rendering/cubemapFragUniforms.json");
	ShaderProgram& cubemapShader;

	Vao debugPointVao;
	Vbo debugPointVbo;
	ShaderProgram& debugPointShader;
};