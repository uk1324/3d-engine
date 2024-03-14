#include "GraphDemo.hpp"
#include <glad/glad.h>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/Utils.hpp>

GraphDemo::GraphDemo() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

template<typename T>
struct Points2 {
	std::vector<T> xs;
	std::vector<T> ys;
};

template<typename T, typename Function>
void computeAntiderivative(Points2<T>& out, Function f, T start, T end, i32 steps) {
	ASSERT(steps % 2 == 0);

	const T step = (end - start) / T(steps);
	T accumulated = 0.0f;

	out.xs.push_back(start);
	out.ys.push_back(accumulated);

	// [0, 2], [2, 4]
	for (i32 i = 0; i < steps; i += 2) {
		const auto x0 = lerp(start, end, T(i) / T(steps));
		const auto x1 = lerp(start, end, T(i + 1) / T(steps));
		const auto x2 = lerp(start, end, T(i + 2) / T(steps));

		// Simpson's rule.
		const auto integralOnX0toX2 = (step / 3.0f) * (f(x0) + T(4) * f(x1) + f(x2));

		accumulated += integralOnX0toX2;
		out.xs.push_back(x2);
		out.ys.push_back(accumulated);
	}
}

template<typename T, typename Function>
void adaptiveIntegrate(Function f, T startT, T endT) {
	const T initialCondition = 123.0f;
	const T maxStep = 0.1f;
	const T minStep = 0.001f;
	const T tolerance = 0.01f;

	T x = initialCondition;
	T t = startT;
	auto step = maxStep;
	bool flag = true;
	while (flag) {
		const T k1 = step * f(t, x);
		const T k2 = step * f(t + (T(1) / T(4)) * step, x + (T(1) / T(4)) * k1);
		const T k3 = step * f(t + (T(1) / T(4)))
	}
}

template<typename T, typename Function>
void plotAntiderivative(Function function) {
	const auto plotRect = ImPlot::GetPlotLimits();
	Points2<T> points;
	computeAntiderivative(points, function, 0.0, plotRect.X.Max, 200);
	ImPlot::PlotLine("antiderivative", points.xs.data(), points.ys.data(), points.xs.size());
	points.xs.clear();
	points.ys.clear();
	computeAntiderivative(points, function, 0.0, plotRect.X.Min, 200);
	ImPlot::PlotLine("antiderivative", points.xs.data(), points.ys.data(), points.xs.size());
}

template<typename Function>
void graph(const char* name, Function f, double minX = -1.0, double maxX = -1.0, i32 sampleCount = -1) {

	const auto plotRect = ImPlot::GetPlotLimits();
	auto clamp = [&](double x) {
		return std::clamp(x, plotRect.X.Min, plotRect.X.Max);
	};
	double min, max;
	if (minX == -1.0f || maxX == -1.0f) {
		min = plotRect.X.Min;
		max = plotRect.X.Max;
	} else {
		min = clamp(minX);
		max = clamp(maxX);
		// TODO: What when min >= max?
		ASSERT(min >= max);
	}

	std::vector<double> xs;
	std::vector<double> ys;
	// @Performance: Could use a sinle array with double stride.
	if (sampleCount == -1) {
		const auto defaultSampleCount = 300;
		sampleCount = defaultSampleCount;
	}

	const double step = (max - min) / double(sampleCount - 1);
	for (i32 i = 0; i <= sampleCount - 1; i++) {
		const double t = double(i) / double(sampleCount - 1);
		const double x = lerp(min, max, t);
		const double y = f(x);
		if (std::abs(x - 0.0f) < 0.01f) {
			int x = 5;
		}
		xs.push_back(x);
		ys.push_back(y);
	}
	ImPlot::PlotLine(name, xs.data(), ys.data(), xs.size());
}

void GraphDemo::update() {
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

	//ImPlot::ShowDemoWindow();

	Begin("plot", nullptr, ImGuiWindowFlags_NoTitleBar); 
	/*const auto PI = 3.14;
	static double xs1[360], ys1[360];
	for (int i = 0; i < 360; ++i) {
		double angle = i * 2 * PI / 359.0;
		xs1[i] = cos(angle); ys1[i] = sin(angle);
	}*/
	float xs2[] = { -1,0,1,0,-1 };
	float ys2[] = { 0,1,0,-1,0 };
	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		ImPlot::SetupAxis(ImAxis_X1);
		ImPlot::SetupAxis(ImAxis_Y1);
		//ImPlot::PlotLine("Circle", xs1, ys1, 360);
		//ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
		//ImPlot::PlotLine("Diamond", xs2, ys2, 5);
		auto function = [](double x) { 
			//return (x - 1.0f) * (x - 2.0) * x;
			return sin(x);
		};

		graph("test", function);
		graph("test", [](double x) { return -cos(x) + 1.0; });
		plotAntiderivative<double>(function);
		//plotAntiderivative(function)


		ImPlot::EndPlot();
	}
	//auto& [xs, ys] = points;

	//if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f)), ImPlotFlags_Equal) {
	//	/*ImPlot::SetupAxes(nullptr, nullptr);
	//	ImPlot::SetupAxisScale(ImGuiAxis_X, ImPlotScale_Linear);
	//	ImPlot::SetupAxisScale(ImGuiAxis_Y, ImPlotScale_Linear);*/
	//	//ImPlot::SetupAxes(nullptr, nullptr);
	//	//ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
	//	//ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);
	//	//ImPlot::SetupAxis(ImAxis_X2, nullptr, ImPlotAxisFlags_AuxDefault);
	//	//ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AuxDefault);

	//	graph("test", [](double x) { return sin(x); });

	//	ImPlot::EndPlot();
	//}

	End();

	Begin("plot settings");

	End();
}