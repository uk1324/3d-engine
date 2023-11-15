#include <water_simulation/AdvectionDemo.hpp>
#include <dependencies/imgui/imgui.h>
#include <Assertions.hpp>
#include <glad/glad.h>

/*
\frac{\partial q}{\partial t} = -u\frac{\partial q}{\partial x} \\
\frac{x_i^{t+1} - x_i^t}{\Delta t} = - u\frac{q_{i+1}^t - q_{i-1}^t}{\Delta x} \\
x_i^{t+1} - x_i^t = -u \Delta t \frac{q_{i+1}^t - q_{i-1}^t}{\Delta x} \\
x_i^{t+1} = x_i^t -u \Delta t \frac{q_{i+1}^t - q_{i-1}^t}{\Delta x}
*/

float triangleFunction(float x) {
	if (x < -1.0f) {
		return 0.0f;
	}
	if (x < 0.0f) {
		return x + 1.0f;
	}
	if (x < 1.0f) {
		return -x + 1.0f;
	}
	return 0.0f;
};

float stepFunction(float x) {
	if (x <= 0.0f) {
		return 1.0f;
	}
	return 0.0f;
}

AdvectionDemo AdvectionDemo::make() {
	auto value = AdvectionDemo{

	};

	value.reset();

	return value;
}

void AdvectionDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));

	ImGui::Begin("advection", nullptr, ImGuiWindowFlags_NoTitleBar);

	ImGui::Combo("integration method", &currentIntegrationMethod, integrationMethodNames);

	ImGui::NewLine();

	ImGui::Text("Reset options");
	ImGui::Combo("initial state", &currentInitialState, initialStateNames);
	if (ImGui::Button("reset")) {
		reset();
	}

	ImGui::NewLine();

	ImGui::Checkbox("paused", &paused);
	if (paused) {
		if (ImGui::Button("step")) {
			runSingleStep = true;
		}

		if (runSingleStep) {
			runStep();
			runSingleStep = false;
		}
	} else {
		runStep();
	}

	ImGui::PlotLines("plot", advectedQuantity.data(), advectedQuantity.size(), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(-1.0f, -1.0f));

	ImGui::End();
}

void AdvectionDemo::reset() {
	advectedQuantity.resize(SIZE, 0.0f);
	newAdvectedQuantity.resize(SIZE);

	const auto offset = 1.1f;
	for (int i = 0; i < advectedQuantity.size(); i++) {
		const auto x = i / static_cast<float>(SIZE) * scale - offset;
		switch (static_cast<InitialState>(currentInitialState)) {
			using enum InitialState;

		case TRIANGLE:
			advectedQuantity[i] = triangleFunction(x);
			break;

		case STEP:
			advectedQuantity[i] = stepFunction(x);
			break;
		}
	}

	// Boundary conditions are handled by not changing the initial values.
	newAdvectedQuantity[0] = advectedQuantity[0];
	newAdvectedQuantity[SIZE - 1] = advectedQuantity[SIZE - 1];
}

void AdvectionDemo::runStep() {
	switch (static_cast<IntegrationMethod>(currentIntegrationMethod)) {
		using enum IntegrationMethod;

	case FORWARD_EULER_CENTRAL_DIFFERENCE:
		// Even though central difference is O(dx^2) accurate it gives the wrong values for some cases for example if the values are
		// 0 a 0
		// Then central difference would give derivative 0. It doesn't work well with high frequency data. It advects it seperately from the other data which causes a lot of oscillation.
		// TODO: Maybe try setting the initial state to (-1)^n.
		for (int i = 1; i < SIZE - 1; i++) {
			newAdvectedQuantity[i] = advectedQuantity[i] - velocity * dt * (advectedQuantity[i + 1] - advectedQuantity[i - 1]) / dx;
		}
		break;

	case FORWARD_EULER_FORWARD_DIFFERENCE:
		for (int i = 1; i < SIZE - 1; i++) {
			newAdvectedQuantity[i] = advectedQuantity[i] - velocity * dt * (advectedQuantity[i + 1] - advectedQuantity[i]) / dx;
		}
		break;

	case FORWARD_EULER_BACKWARD_DIFFERENCE:
		// 
		for (int i = 1; i < SIZE - 1; i++) {
			newAdvectedQuantity[i] = advectedQuantity[i] - velocity * dt * (advectedQuantity[i] - advectedQuantity[i - 1]) / dx;
		}
		break;
	}

	for (int i = 0; i < advectedQuantity.size(); i++) {
		advectedQuantity[i] = newAdvectedQuantity[i];
	}
}
