#pragma once
#include <game/FpsController.hpp>
#include <framework/Instancing.hpp>
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

	struct Config {
		float dt;
		float gravity = 9.81f;
		float viscousDrag = 0.0f;
	};
	void step(const Config& config);

	i64 gridSizeX() const;
	i64 gridSizeY() const;
	Vec2T<i64> gridSize() const;

	float totalHeight() const;

	// Storing the size thrice for no reason.
	Array2d<float> height;
	Array2d<float> velX;
	Array2d<float> velY;

	// Is it better to seperate the configuration state and the update state. In cases where there might be multiple shallow water simulations storing a single config struct instead of one per simulation might be good. Then the config struct would just be passed to functions.
	// Instead of using a struct could just pass the arguments normally.

	float gridCellSize;
};

struct MainLoop {
	static MainLoop make();
	void update();

	bool paused = false;
	float elapsed = 0.0f;
	i64 framesElapsed = 0;

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