#include <water_simulation/IntegrationDemo.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/Utils.hpp>
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Color.hpp>
#include <imgui/implot.h>
#include <random>
#include "glad/glad.h"
#include <Span.hpp>
#include <array>

template<typename Function>
void plotFunction(
	Function f, 
	Vec3 color = Color3::WHITE, 
	float start = -std::numeric_limits<float>::infinity(),
	float end = std::numeric_limits<float>::infinity()) {
	const auto plotRect = ImPlot::GetPlotLimits();
	std::vector<float> xs;
	std::vector<float> ys;

	start = std::clamp(start, float(plotRect.X.Min), float(plotRect.X.Max));
	end = std::clamp(end, float(plotRect.X.Min), float(plotRect.X.Max));

	const auto count = 400;
	for (int i = 0; i < count; i++) {
		const float t = (i / float(count - 1));
		float x = lerp(start, end, t);
		float y = f(x);
		xs.push_back(x);
		ys.push_back(y);
	}
	ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color.x, color.y, color.z, 1.0f));
	ImPlot::PlotLine("test", xs.data(), ys.data(), xs.size());
	ImPlot::PopStyleColor();
}

IntegrationDemo::IntegrationDemo() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

template<typename T>
T polynomialEvaluateHorner(Span<const T> coefficients, T x) {
	if (coefficients.size() == 0) {
		return 0.0f;
	}
	T output = coefficients[coefficients.size() - 1];
	for (i64 i = i64(coefficients.size() - 2); i >= 0; i--) {
		output = output * x + coefficients[i];
	}
	return output;
}

std::array<double, 3> interpolate(Vec2 v0, Vec2 v1, Vec2 v2) {
	const double a0 = v0.x, b0 = v0.y, a1 = v1.x, b1 = v1.y, a2 = v2.x, b2 = v2.x;
	const auto a = (-a0 + b0) / (a0 * a0 - a0 * a1 - a0 * a2 + a1 * a2);
	const auto b = (a0 * a0 + a1 * a2 - a1 * b0 - a2 * b0) / (a0 * a0 - a0 * a1 - a0 * a2 + a1 * a2);
	const auto c = (-a0 * a1 * a2 + a1 * a2 * b0) / (a0 * a0 - a0 * a1 -
		a0 * a2 + a1 * a2);
	return { a, b, c };
}

double evaluateInterpolatingQuadratic(double x, Vec2 v0, Vec2 v1, Vec2 v2) {
	return ((x - v1.x) * (x - v2.x)) / ((v0.x - v1.x) * (v0.x - v2.x)) * v0.y +
		((x - v0.x) * (x - v2.x)) / ((v1.x - v0.x) * (v1.x - v2.x)) * v1.y +
		((x - v0.x) * (x - v1.x)) / ((v2.x - v0.x) * (v2.x - v1.x)) * v2.y;
}

struct SimpsonsRuleOutput {
	double inegral;
};

template<typename Function>
SimpsonsRuleOutput simpsonsRule(Function f, double start, double end, i32 steps) {
	ASSERT(steps % 2 == 0);
	const double step = (end - start) / steps;

	// Split the sum into 3 parts to improve rounding error bounds.
	double s0 = f(start) + f(end);
	double s1 = 0;
	double s2 = 0;

	// Could split this into 2 loops. Could vectorize this.
	for (i32 i = 1; i <= steps - 1; i++) {
		const double x = start + i * step;
		if (i % 2 == 0) {
			s2 += f(x);
		} else {
			s1 += f(x);
		}
	}

	return SimpsonsRuleOutput{
		.inegral = step * (s0 + 2.0 * s2 + 4.0 * s1) / 3.0,
	};
}

Vec3 colorFromNumber(i64 i) {
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	std::default_random_engine engine(i);
	/*return Color3::fromHsv(distribution(engine), 1.0f, 1.0f);*/
	/*return Color3::fromHsv((i * 31 % 7) / float(7), 1.0f, 1.0f);*/
	return Color3::fromHsv((i * 103 % 7) / float(7), 1.0f, 1.0f);
}

void IntegrationDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	using namespace ImGui;

	auto id = DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoTabBar);

	static bool firstFrame = true;
	if (firstFrame) {
		DockBuilderRemoveNode(id);
		DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

		DockBuilderDockWindow("plot", rightId);
		DockBuilderDockWindow("plot settings", leftId);

		DockBuilderFinish(id);
		firstFrame = false;
	}

	Begin("plot", nullptr, ImGuiWindowFlags_NoTitleBar);



	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f))) {
		plotFunction([this](double x) { return f(x); }, Color3::WHITE);
		simpsonsMethod();
		ImPlot::EndPlot();
	}

	End();

	Begin("plot settings");
	ImGui::InputFloat("start", &start);
	ImGui::InputFloat("end", &end);
	ImGui::SliderInt("step count", &simpsonsMethodStepCount, 2, 20);
	End();
}

void IntegrationDemo::simpsonsMethod() {
	float step = (end - start) / simpsonsMethodStepCount;
	std::vector<double> xs;
	std::vector<double> ys;
	for (i32 i = 0; i < simpsonsMethodStepCount; i += 2) {
		const auto x0 = start + i * step;
		const auto x1 = start + (i + 1) * step;
		const auto x2 = start + (i + 2) * step;
		const auto y0 = f(x0);
		const auto y1 = f(x1);
		const auto y2 = f(x2);
		xs.push_back(x0);
		xs.push_back(x1);
		xs.push_back(x2);
		ys.push_back(y0);
		ys.push_back(y1);
		ys.push_back(y2);
		plotFunction([&](double x) {
			return evaluateInterpolatingQuadratic(x, Vec2(x0, y0), Vec2(x1, y1), Vec2(x2, y2));
		}, colorFromNumber(i), x0, x2);
	}
	ImPlot::PlotScatter("points", xs.data(), ys.data(), xs.size());
}

double IntegrationDemo::f(double x) {
	return sin(x) * exp(-x * 0.3f);
}
