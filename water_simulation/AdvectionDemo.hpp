#pragma once

#include <Types.hpp>
#include <vector>

struct AdvectionDemo {
	static AdvectionDemo make();

	void update();

	void reset();
	void runStep();

	enum class InitialState {
		TRIANGLE,
		STEP
	};
	static constexpr const char* initialStateNames =
		"triangle\0"
		"step\0";

	enum class IntegrationMethod {
		// name = time integration + space differentiation
		FORWARD_EULER_CENTRAL_DIFFERENCE,
		FORWARD_EULER_FORWARD_DIFFERENCE,
		FORWARD_EULER_BACKWARD_DIFFERENCE,
	};
	static constexpr const char* integrationMethodNames =
		"forward euler central difference\0"
		"forward euler forward difference\0"
		"forward euler backward difference\0";

	int currentIntegrationMethod = static_cast<int>(IntegrationMethod::FORWARD_EULER_CENTRAL_DIFFERENCE);
	int currentInitialState = static_cast<int>(InitialState::TRIANGLE);

	bool paused = false;
	bool runSingleStep = false;


	static const i64 SIZE = 200;
	static constexpr float scale = 10.0f;
	float dt = 1.0f / 600.0f;
	float dx = scale / SIZE;
	float velocity = 1.0f;

	std::vector<float> advectedQuantity;
	std::vector<float> newAdvectedQuantity;
};