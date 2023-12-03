#include <water_simulation/AdvectionDemo.hpp>
#include <dependencies/imgui/imgui.h>
#include <engine/Math/FourierTransform.hpp>
#include <engine/Math/Angles.hpp>
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

#include <iostream>
#include <Put.hpp>
#include <span>

template<typename T>
void putSpan(std::span<const T> span) {
	std::cout << "[";
	for (i64 i = 0; i < static_cast<i64>(span.size()) - 1; i++) {
		std::cout << span[i] << "; ";
	}
	if (span.size() > 0) {
		std::cout << span[span.size() - 1];
	}
	std::cout << "]";
}

template<typename T>
std::span<const T> vectorConstSpan(const std::vector<T>& v) {
	return std::span<const T>(v);
}

template<typename T>
void putVector(const std::vector<T>& vector) {
	putSpan(vectorConstSpan(vector));
}

#include <algorithm>

template <typename _Real>
static inline
void rotshift(std::complex<_Real>* complexVector, const size_t count)
{
	int center = (int)floor((float)count / 2);
	if (count % 2 != 0) {
		center++;
	}
	// odd: 012 34 changes to 34 012
	std::rotate(complexVector, complexVector + center, complexVector + count);
}

template <typename _Real>
static inline
void irotshift(std::complex<_Real>* complexVector, const size_t count)
{
	int center = (int)floor((float)count / 2);
	// odd: 01 234 changes to 234 01
	std::rotate(complexVector, complexVector + center, complexVector + count);
}

void calculateSpectralDerivative(std::vector<std::complex<double>>& inOut, double domainLength) {
	std::vector<std::complex<double>> scales;
	scales.resize(inOut.size());

	for (int i = 0; i < inOut.size(); i++) {
		auto& scale = scales[i];
		scale = ((i / static_cast<double>(inOut.size())) - 0.5) * inOut.size();
		scale *= 2.0 * PI<double> / domainLength;
	}
	rotshift(scales.data(), scales.size());

	dft(inOut);
	for (int i = 0; i < inOut.size(); i++) {
		inOut[i] *= scales[i] * std::complex(0.0, 1.0);
	}
	inverseDft(inOut);
	/*put("kappa shifted");
	
	putVector(kappa);
	putNewline();

	for (int i = 0; i < a.size(); i++) {
		a[i] *= kappa[i] * std::complex(0.0, 1.0);
	}*/
}

void spectralDerivativeTest() {
	std::vector<std::complex<double>> a{ 1.0, 2.0, 3.0, 4.0 };
	auto aCopy = a;
	const auto n = a.size();
	const auto l = 2.0;

	dft(a);
	put("transformed");
	putVector(a);
	putNewline();

	std::vector<std::complex<double>> kappa;
	kappa.resize(n);

	for (int i = 0; i < n; i++) {
		kappa[i] = ((i / static_cast<double>(n)) - 0.5) * n;
	}

	put("kappa scaled");
	for (int i = 0; i < n; i++) {
		kappa[i] *= 2.0 * PI<double> / l;
	}
	putVector(kappa);
	putNewline();

	put("kappa shifted");
	rotshift(kappa.data(), kappa.size());
	putVector(kappa);
	putNewline();

	for (int i = 0; i < a.size(); i++) {
		a[i] *= kappa[i] * std::complex(0.0, 1.0);
	}
	put("derivative fft\n");
	putVector(a);
	putNewline();

	inverseDft(a);

	put("derivative 1");
	putVector(a);
	putNewline();

	put("derivative 2");
	calculateSpectralDerivative(aCopy, l);
	putVector(aCopy);
	putNewline();
}

AdvectionDemo AdvectionDemo::make() {
	auto value = AdvectionDemo{

	};
	//spectralDerivativeTest();
	value.reset();

	return value;
}

void plotVector(const std::vector<float>& vector) {
	ImGui::PlotLines("plot", vector.data(), vector.size(), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(-1.0f, -1.0f));;
}

void plotVector(const char* name, const std::vector<double>& vector) {
	std::vector<float> a;
	a.resize(vector.size());
	for (int i = 0; i < a.size(); i++) {
		a[i] = vector[i];
	}

	ImGui::PlotLines(name, a.data(), a.size(), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(-1.0f, -1.0f));;
}

//void calculateSpectralDerivative(std::vector<std::complex<double>>& inOut) {
//
//}

void AdvectionDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	//const auto n = 128;
	//const auto l = 30.0;
	//std::vector<double> fs;
	//std::vector<double> dfs;
	//auto dx = l / n;
	//
	//fs.resize(n);
	//dfs.resize(n);
	//for (int i = 0; i < n; i++) {
	//	const auto x = ((static_cast<double>(i) / static_cast<double>(n)) - 0.5) * l;
	//	fs[i] = cos(x) * exp(-pow(x, 2.0) / 25.0);
	//	dfs[i] = -(sin(x) * exp(-pow(x, 2.0) / 25.0) + (2.0 / 25.0) * x * fs[i]);
	//}
	////ImGui::Begin("b");
	//////plotVector("bv", fs);
	////ImGui::End();

	//std::vector<std::complex<double>> transformed;
	//transformed.resize(n);
	//for (int i = 0; i < n; i++) {
	//	transformed[i] = fs[i];
	//}

	//calculateSpectralDerivative(transformed, l);
	////dft(transformed);
	////for (int i = 0; i < transformed.size(); i++) {
	////	transformed[i] = static_cast<double>(i) * (TAU<double> / l) * std::complex(0.0, 1.0) * transformed[i];
	////}
	////inverseDft(transformed);
	//////calculateSpectralDerivative(transformed);

	//std::vector<double> derivative;
	//derivative.resize(n);

	//for (int i = 0; i < n; i++) {
	//	/*derivative[i] = transformed[i].real() - dfs[i];*/
	//	derivative[i] = transformed[i].real();
	//}
	////ImGui::Begin("a");
	////plotVector("av", fs);
	////ImGui::End();

	//ImGui::Begin("b");
	//plotVector("bv", dfs);
	//ImGui::End();

	//ImGui::Begin("c");
	//plotVector("cv", derivative);
	//ImGui::End();

	///*rotshift(spectralDerivativeTemp.data(), spectralDerivativeTemp.size());
	//for (int i = 0; i < spectralDerivativeTemp.size(); i++) {
	//	spectralDerivativeTemp[i] *= std::complex(0.0, 1.0) * static_cast<double>(i) * (PI<double> / (static_cast<double>(dx * advectedQuantity.size())));
	//}
	//inverseDft(spectralDerivativeTemp);*/

	//for (int i = 0; i < spectralDerivativeTemp.size(); i++) {
	//	newAdvectedQuantity[i] = advectedQuantity[i] - velocity * dt * static_cast<float>(spectralDerivativeTemp[i].real());
	//}

	//return;

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
	spectralDerivativeTemp.resize(SIZE);
	spectralDerivativeWithIterpolatedDataTemp.resize(SIZE * 2);

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

		case GAUSSIAN:
			advectedQuantity[i] = exp(-pow(x - 0.5f, 2.0f) * 2.0f);
			break;
		}
	}

	// Boundary conditions are handled by not changing the initial values.
	if (static_cast<InitialState>(currentInitialState) == InitialState::GAUSSIAN) {
		advectedQuantity[0] = 0.0f;
	}
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

	case FORWARD_EULER_SPECTRAL_DERIVATIVE: {
		auto& temp = spectralDerivativeTemp;
		for (int i = 0; i < temp.size(); i++) {
			temp[i] = advectedQuantity[i];
		}
		calculateSpectralDerivative(temp, dx * advectedQuantity.size());

		for (int i = 0; i < temp.size(); i++) {
			newAdvectedQuantity[i] = advectedQuantity[i] - velocity * dt * static_cast<float>(temp[i].real());
		}
		break;
	}
		

	case FORWARD_EULER_SPECTRAL_DERIVATIVE_WITH_INTERPOLATED_DATA: {
		auto& temp = spectralDerivativeWithIterpolatedDataTemp;
		for (int i = 0; i < temp.size(); i++) {
			temp[i] = 0.0;
		}
		for (int i = 0; i < advectedQuantity.size() - 1; i++) {
			temp[i * 2] = advectedQuantity[i];
			temp[i * 2 + 1] = (advectedQuantity[i] + advectedQuantity[i + 1]) / 2.0;
		}
		/*std::vector<double> test;
		test.resize(temp.size());*/
		calculateSpectralDerivative(temp, dx * advectedQuantity.size());
		/*for (int i = 0; i < test.size(); i++) {
			test[i] = temp[i].real();
		}
		ImGui::Begin("aaa");
		plotVector("name", test);
		ImGui::End();*/

		for (int i = 0; i < advectedQuantity.size(); i++) {
			//newAdvectedQuantity[i] = temp[i * 2].real();
			newAdvectedQuantity[i] = advectedQuantity[i] - velocity * dt * static_cast<float>(temp[i * 2].real());
		}
		//for (int i = 0; i < advectedQuantity.size(); i++) {
		//	newAdvectedQuantity[i] = advectedQuantity[i];
		//}
		break;
	}
		
	}



	for (int i = 0; i < advectedQuantity.size(); i++) {
		advectedQuantity[i] = newAdvectedQuantity[i];
	}
}
