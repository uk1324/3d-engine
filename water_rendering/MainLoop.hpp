#pragma once
#include <game/FpsController.hpp>
#include <water_rendering/Instancing.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/Texture.hpp>
#include <water_rendering/SavableUniforms.hpp>
#include <water_rendering/Shaders/cubemapShaderData.hpp>
#include <water_rendering/Shaders/waterShaderData.hpp>
#include <water_rendering/Shaders/SkyboxSettingsData.hpp>

struct MainLoop {
	static MainLoop make();
	void update();

	bool paused = false;
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

	SerializedSettings<SkyboxSettings> skyboxSettings{ "water_rendering/skyboxSettings.json" };

	SerializedSettings<CubemapShaderFragUniforms> cubemapShaderFragUniforms{ "water_rendering/cubemapFragUniforms.json" };

	SerializedSettings<WaterShaderVertUniforms> waterShaderVertUniforms{ "water_rendering/waterShaderVertUniforms.json" };
	SerializedSettings<WaterShaderFragUniforms> waterShaderFragUniforms{ "water_rendering/waterShaderFragUniforms.json" };

	ShaderProgram& cubemapShader;

	Vao heightMapVao;
	Vbo heightMapVbo;
	ShaderProgram& heightMapShader;
	Texture heightMap;

	Vao debugPointVao;
	Vbo debugPointVbo;
	ShaderProgram& debugPointShader;
	
	static constexpr i64 GRID_SIZE = 100;
	float height[GRID_SIZE][GRID_SIZE]{ 0.0f };
	//float dhdt[GRID_SIZE][GRID_SIZE]{ 0.0f };
	Vec2 vel[GRID_SIZE][GRID_SIZE];

	u32 ubo;
};