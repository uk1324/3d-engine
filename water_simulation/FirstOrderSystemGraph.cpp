#include <water_simulation/FirstOrderSystemGraph.hpp>
#include <imgui/implot.h>
#include <algorithm>

FirstOrderSystemGraph::FirstOrderSystemGraph() {
}

void FirstOrderSystemGraph::update() {
}

void FirstOrderSystemGraph::plot() {
	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		//ImPlot::SetupAxesLimits(-2.0f, 2.0f, -2.0f, 2.0f);
		ImPlot::SetupAxis(ImAxis_X1);
		ImPlot::SetupAxis(ImAxis_Y1);

		ImPlot::EndPlot();
	}
}

#include <Gui.hpp>

void FirstOrderSystemGraph::plotSettings() {
	auto editor = [&]() {
		if (Gui::inputText("x' = ", formulaInput, std::size(formulaInput))) {
			
		}
	};

	GUI_PROPERTY_EDITOR({ editor(); });
	//ImGui::InputText("x'", formulaInput, std::size(formulaInput));
}
