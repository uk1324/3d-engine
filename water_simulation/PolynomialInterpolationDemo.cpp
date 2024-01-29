#include "PolynomialInterpolationDemo.hpp"
#include <glad/glad.h>
#include <imgui/implot.h>

PolynomialInterpolationDemo::PolynomialInterpolationDemo() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void PolynomialInterpolationDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	using namespace ImGui;

	DockSpaceOverViewport(ImGui::GetMainViewport());

	//DockSpaceOverViewport(ImGui::GetMainViewport());
	ShowDemoWindow();
	ImPlot::ShowDemoWindow();

	Begin("plot window");

	ImPlot::BeginPlot("main plot", ImVec2(-1.0f, -1.0f));

	float xs[] = { 1.0f, 3.0f, 2.0f, 4.0f };
	float ys[] = { 1.0f, 3.0f, 2.0f, 4.0f };

	ImPlot::PlotScatter("test", xs, ys, 4);

	ImPlot::EndPlot();

	End();
}
