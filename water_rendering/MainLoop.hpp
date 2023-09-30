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
#include <engine/Utils/Array2d.hpp>

struct ShallowWaterSimulation {
	ShallowWaterSimulation(i64 gridSizeX, i64 gridSizeY, float gridCellSize);

	void step(float dt, float gravity = 9.81f);

	i64 gridSizeX() const;
	i64 gridSizeY() const;
	Vec2T<i64> gridSize() const;

	// Storing the size thrice for no reason.
	Array2d<float> height;
	Array2d<float> velX;
	Array2d<float> velY;

	float gridCellSize;
};

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
	
	ShallowWaterSimulation shallowWaterSimulation;

	u32 ubo;
};