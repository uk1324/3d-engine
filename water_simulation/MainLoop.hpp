#pragma once

#include <engine/Graphics/Texture.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Vbo.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <water_simulation/Fluid.hpp>
#include <Array2d.hpp>
#include <framework/Renderer2d.hpp>

struct MainLoop {
	static MainLoop make();

	float dt = 1.0f / 60.0f;
	float elapsed = 0.0f;

	EulerianFluid fluid;
	Array2d<float> smokeR, smokeG, smokeB;
	void setSmoke(i64 x, i64 y, Vec3 color);
	Image32 image;
	Renderer2d renderer;

	bool paused = false;
	float brushRadiusCellCount = 4.0f;

	bool velocityBrushAddSmoke = false;
	bool cycleColors = false;
	float colorCyclingSpeed = 1.0f;
	Vec3 velocityBrushSmokeColor = Vec3(0.0f);

	Vec2 velocityBrushPos;
	bool velocityBrushReleased = true;

	bool useDefaultDisplayedPressureBounds = true;
	float displayedPressureMin = 0.0f;
	float displayedPressureMax = 0.0f;

	enum class DisplayState {
		SMOKE,
		DIVERGENCE,
		PRESSURE,
	};
	DisplayState displayState = DisplayState::SMOKE;
	const char* displayStateNames = "smoke\0divergence\0pressure\0";

	enum class Scene {
		BOX,
		WIND_TUNNEL,
	};
	Scene scene = Scene::BOX;
	const char* sceneNames = "box\0wind tunnel\0";

	float windTunnelVelocity = 2.0f;

	std::vector<Vec2> particles;
	std::optional<float> initialArea;

	void update();
};