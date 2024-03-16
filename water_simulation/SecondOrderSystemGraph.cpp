#include <water_simulation/SecondOrderSystemGraph.hpp>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Input/Input.hpp>

SecondOrderSystemGraph::SecondOrderSystemGraph() {
	plotCompiler.formulaInputs.push_back(&xFormulaInput);
	plotCompiler.formulaInputs.push_back(&yFormulaInput);
}

void plotLine(const char* label, const std::vector<Vec2>& vs) {
	const auto pointsData = reinterpret_cast<const float*>(vs.data());
	ImPlot::PlotLine(label, pointsData, pointsData + 1, vs.size(), 0, 0, sizeof(float) * 2);
}

void SecondOrderSystemGraph::update() {
	auto id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	const auto derivativePlotWindowName = "v' plot";
	const auto settingsWindowName = "settings";

	static bool firstFrame = true;
	if (firstFrame) {
		ImGui::DockBuilderRemoveNode(id);
		ImGui::DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

		ImGui::DockBuilderDockWindow(derivativePlotWindowName, rightId);
		ImGui::DockBuilderDockWindow(settingsWindowName, leftId);

		ImGui::DockBuilderFinish(id);
		firstFrame = false;
	}

	ImGui::Begin(derivativePlotWindowName);
	derivativePlot();
	ImGui::End();

	ImGui::Begin(settingsWindowName);
	settings();
	ImGui::End();
}

void SecondOrderSystemGraph::derivativePlot() {
	auto calculateDerivative = [this](Vec2 v, float t) -> Vec2 {
		//return Vec2(x.y, x.x - pow(x.x, 3.0f) - x.y * damping);
		return Vec2(v.y, sin(v.x));
		//return Vec2(v.y, sin(v.x) - v.x + 2.0f - v.y * damping);
		//return Vec2(x.y, pow(x.x, 2.0f) * pow(x.y, 2.0f));
		//return Vec2(x.y, x.x + x.y + 1.0f);
		//return Vec2(x.y, x.x - pow(x.x, 3.0f) - x.y);
		/*return Vec2(v.y, sin(v.x) - v.y * damping);*/
		//return Vec2(v.y, -v.x);
		//return Vec2(cos(x.y), sin(x.x));
	};
	
	if (!paused) {
		float dt = 1.0f / 60.0f;
		for (auto& point : testPoints) {
			point.history.push_back(point.pos);
			point.pos = rungeKutta4Step(calculateDerivative, point.pos, 0.0f, dt);
		}
	}
	
	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		ImPlot::SetupAxis(ImAxis_X1);
		ImPlot::SetupAxis(ImAxis_Y1);
	
		const auto plotRect = ImPlot::GetPlotLimits();
		// TODO: To find the fixed points could compute the abs() of all the values then find the local minima. If the minima are near zero then I could run root finding.
		// TODO: Could use NaN to make breaks in lines.
	
		const auto minX = i32(floor(plotRect.X.Min / spacing));
		const auto minY = i32(floor(plotRect.Y.Min / spacing));
		const auto maxX = i32(ceil(plotRect.X.Max / spacing));
		const auto maxY = i32(ceil(plotRect.Y.Max / spacing));
		points.clear();
		for (i32 xi = minX; xi <= maxX; xi++) {
			for (i32 yi = minY; yi <= maxY; yi++) {
				const auto x = xi * spacing;
				const auto y = yi * spacing;
	
				auto current = Vec2(x, y);
				const auto step = spacing / 5;
				Vec2 derivative;
				for (int i = 0; i < 15; i++) {
					derivative = calculateDerivative(current, 0.0f);
					const auto newPos = current + derivative * step;
					points.push_back(current);
					points.push_back(newPos);
					current = newPos;
				}
				const auto angle = derivative.angle();
				const auto a = Vec2::oriented(angle + 0.4f);
				const auto b = Vec2::oriented(angle - 0.4f);
				points.push_back(current);
				points.push_back(current - a.normalized() * 0.01f);
				points.push_back(current);
				points.push_back(current - b.normalized() * 0.01f);
			}
		}
		const auto pointsData = reinterpret_cast<float*>(points.data());
		ImPlot::PlotLine("testa", pointsData, pointsData + 1, points.size(), ImPlotLineFlags_Segments, 0, sizeof(Vec2));
	
		{
			const auto testPointsData = reinterpret_cast<const u8*>(testPoints.data());
			ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImPlot::PlotScatter(
				"testPoints",
				reinterpret_cast<const float*>(testPointsData + offsetof(TestPoint, pos.x)),
				reinterpret_cast<const float*>(testPointsData + offsetof(TestPoint, pos.y)),
				testPoints.size(),
				0,
				0,
				sizeof(TestPoint)
			);
			for (const auto& p : testPoints) {
				plotLine("test", p.history);
			}
			ImPlot::PopStyleColor();
		}
			
		Input::ignoreImGuiWantCapture = true;
		if (Input::isMouseButtonDown(MouseButton::MIDDLE)) {
			const auto mousePos = ImPlot::GetPlotMousePos();
			const auto p = TestPoint{ Vec2(mousePos.x, mousePos.y) };
			testPoints.push_back(p);
		}
		Input::ignoreImGuiWantCapture = false;
	
		ImPlot::EndPlot();
	}
}

void SecondOrderSystemGraph::settings() {
	plotCompiler.formulaInputGui("x'=", xFormulaInput);
	plotCompiler.formulaInputGui("y'=", yFormulaInput);

	ImGui::Checkbox("paused", &paused);
}
