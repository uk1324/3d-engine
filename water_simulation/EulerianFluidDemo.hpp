#pragma once

#include <water_simulation/EulerianFluid.hpp>
#include <engine/Math/MarchingSquares.hpp>
#include <framework/Renderer2d.hpp>
#include <Array2d.hpp>

struct EulerianFluidDemo {
	static EulerianFluidDemo make();

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

	bool drawStreamlines = false;

	enum class Scene {
		BOX,
		WIND_TUNNEL,
	};
	Scene scene = Scene::BOX;
	const char* sceneNames = "box\0wind tunnel\0";

	float windTunnelVelocity = 2.0f;

	std::vector<Vec2> particles;
	std::optional<float> initialArea;

	enum class IntegrationMethod {
		EULER,
		RK4,
	};
	const char* integrationMethodNames= "euler\0rk4\0";

	struct PathlineParticle {
		Vec2 pos;
		Vec3 color;
		IntegrationMethod integrationMethod;
		std::vector<Vec2> positionHistory;
	};
	std::vector<PathlineParticle> pathlineParticles;

	struct Isobar {
		Vec3 color;
		float value;
	};
	bool drawAutomaticIsobars = false;
	float automaticIsobarsMin = -100.0f;
	float automaticIsobarsMax = 100.0f;
	float automaticIsobarsStep = 20.0f;
	std::vector<Isobar> isobars;
	std::vector<MarchingSquaresLine> marchingSquaresOutput;

	void update();
};