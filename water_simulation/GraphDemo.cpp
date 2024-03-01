#include "GraphDemo.hpp"
#include <glad/glad.h>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/Utils.hpp>

GraphDemo::GraphDemo() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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

	ImPlot::ShowDemoWindow();

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
		graph("test", [](double x) { return sin(x); });

		ImPlot::EndPlot();
	}

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