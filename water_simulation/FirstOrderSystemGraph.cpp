#include "FirstOrderSystemGraph.hpp"
#include "FirstOrderSystemGraph.hpp"
#include <water_simulation/FirstOrderSystemGraph.hpp>
#include <dependencies/math-compiler/src/errorMessage.hpp>
#include <imgui/implot.h>
#include <StringUtils.hpp>
#include <engine/Math/Utils.hpp>
#include <Gui.hpp>
#include <algorithm>
#include <water_simulation/assets/IconsFontAwesome5.h>
#include <imgui/imgui_internal.h>
#include <water_simulation/PlotUtils.hpp>
#include <engine/Math/RootFinding/bisection.hpp>
#include <engine/Math/MarchingSquares.hpp>

const auto STABLE_FIXED_POINTS_COLOR = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
const auto UNSTABLE_FIXED_POINTS_COLOR = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

void scatterPlotVec2s(const char* label, const std::vector<Vec2>&vs) {
	const auto pointsData = reinterpret_cast<const float*>(vs.data());
	ImPlot::PlotScatter(label, pointsData, pointsData + 1, vs.size(), 0, 0, sizeof(float) * 2);
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

FirstOrderSystemGraph::FirstOrderSystemGraph() 
	: derivativePlotInput(1)
	, derivativePlotOutput(1) {

	plotCompiler.variables.push_back(Variable{ .name = "x" });
	plotCompiler.formulaInputs.push_back(&formulaInput);
}

void FirstOrderSystemGraph::update() {
	auto id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	const auto derivativePlotWindowName = "derivative plot";
	const auto potentialPlotWindowName = "potential plot";
	const auto settingsWindowName = "settings";

	static bool firstFrame = true;
	if (firstFrame) {
		ImGui::DockBuilderRemoveNode(id);
		ImGui::DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

		ImGui::DockBuilderDockWindow(derivativePlotWindowName, rightId);
		ImGui::DockBuilderDockWindow(potentialPlotWindowName, rightId);
		ImGui::DockBuilderDockWindow(settingsWindowName, leftId);

		ImGui::DockBuilderFinish(id);
		firstFrame = false;
	}

	ImGui::Begin(derivativePlotWindowName);
	derivativePlot();
	ImGui::End();

	ImGui::Begin(potentialPlotWindowName);
	potentialPlot();
	ImGui::End();

	{
		// TODO: Could implement removing via moving the last element into the position and popping the last element. Then if the interation is done from the back no additional space is required.
		// https://stackoverflow.com/questions/39019806/using-erase-remove-if-idiom

		const auto [f, l] = std::ranges::remove_if(bifurcationPlots, [&](const BifurcationPlot& plot) -> bool {
			bool open = true;
			ImGui::Begin(("'" + plot.parameterName + "' bifurcation diagram").c_str(), &open);
			bifurcationPlot(plot.parameterName);
			ImGui::End();
			return !open;
		});
		bifurcationPlots.erase(f, l);
	}
	
	ImGui::Begin(settingsWindowName);
	settings();
	ImGui::End();
}

void FirstOrderSystemGraph::derivativePlot() {
	if (!ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		return;
	}
	ImPlot::SetupAxis(ImAxis_X1, "x");
	ImPlot::SetupAxis(ImAxis_Y1, "x'");

	if (!formulaInput.loopFunction.has_value()) {
		ImPlot::EndPlot();
		return;
	}

	const auto limits = ImPlot::GetPlotLimits();
	const auto min = float(limits.X.Min);
	const auto max = float(limits.X.Max);
	i32 steps = 200;
	derivativePlotInput.reset(plotCompiler.variables.size() + plotCompiler.parameters.size());
	for (i32 i = 0; i <= steps - 1; i++) {
		const auto t = float(i) / float(steps - 1);
		const auto x = lerp(min, max, t);
		plotCompiler.loopFunctionVariablesBlock[0] = x;
		derivativePlotInput.append(plotCompiler.loopFunctionVariablesBlock);
	}
	derivativePlotOutput.resizeWithoutCopy(derivativePlotInput.blockCount_);
	(*formulaInput.loopFunction)(derivativePlotInput, derivativePlotOutput);

	std::vector<float> xs;
	std::vector<float> ys;
	for (i64 i = 0; i < derivativePlotOutput.blockCount_; i++) {
		const float x = derivativePlotInput(i, 0);
		const float y = derivativePlotOutput(i, 0);
		xs.push_back(x);
		ys.push_back(y);
	}

	std::vector<Vec2> stableFixedPoints;
	std::vector<Vec2> unstableFixedPoints;
	for (i64 i = 0; i < i64(ys.size()) - 1; i++) {
		const float y0 = ys[i];
		const float y1 = ys[i + 1];
		const auto oppositeSigns = y0 * y1 < 0.0f;
		if (!oppositeSigns) {
			continue;
		}
		// Not using Newton's method, because I have no derivative. Also it doesn't provide the x interval in which the value is contained; the value may be close to zero, but the distance from the actual zero might be large for example in x^3.
		// Currently zero's that are minima are not handled.
		const auto result = bisect<float>(
			xs[i],
			xs[i + 1],
			[this](float x) { return plotCompiler.callLoopFunctionWithSingleOutput(formulaInput, x); },
			10,
			0.0f // Set to 0.0, because the code doesn't account for zoom so just always do the 2 iterations.
		);
		if (result.result != BisectionResult::SUCCESS && result.result != BisectionResult::MAX_ITERATION_COUNT_EXCEEDED) {
			ASSERT_NOT_REACHED();
			continue;
		}

		const auto x = result.input;
		// Should this just be set to zero or the function value?
		const auto y = 0.0f;
		// Could calculate the stablity after getting the output from root finding. The problem is if the values different then the stability wouldn't agree with what you see in the graph. This probably would probably happen only if the function oscillated quickly around that region.
		if (y0 >= y1) {
			stableFixedPoints.push_back(Vec2(x, y));
		} else {
			unstableFixedPoints.push_back(Vec2(x, y));
		}
	}

	ImPlot::PlotLine("x'", xs.data(), ys.data(), xs.size());

	ImPlot::PushStyleColor(ImPlotCol_MarkerFill, STABLE_FIXED_POINTS_COLOR);
	ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, STABLE_FIXED_POINTS_COLOR);
	scatterPlotVec2s("stable fixed points", stableFixedPoints);
	ImPlot::PopStyleColor();
	ImPlot::PopStyleColor();

	ImPlot::PushStyleColor(ImPlotCol_MarkerFill, UNSTABLE_FIXED_POINTS_COLOR);
	ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, UNSTABLE_FIXED_POINTS_COLOR);
	scatterPlotVec2s("unstable fixed points", unstableFixedPoints);
	ImPlot::PopStyleColor();
	ImPlot::PopStyleColor();

	ImPlot::EndPlot();
}

void FirstOrderSystemGraph::potentialPlot() {
	if (!ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		return;
	}
	ImPlot::SetupAxis(ImAxis_X1, "x");
	ImPlot::SetupAxis(ImAxis_Y1, "potential");

	if (!formulaInput.loopFunction.has_value()) {
		ImPlot::EndPlot();
		return;
	}

	const auto plotRect = ImPlot::GetPlotLimits();
	Points2<float> points;
	auto function = [&](float x) {
		return plotCompiler.callLoopFunctionWithSingleOutput(formulaInput, x);
	};

	// The potential is negated.
	auto flipY = [&points]() {
		for (auto& y : points.ys) {
			y = -y;
		}
	};
	computeAntiderivative<float>(points, function, 0.0, plotRect.X.Max, 200);
	flipY();
	ImPlot::PlotLine("antiderivative", points.xs.data(), points.ys.data(), points.xs.size());
	points.xs.clear();
	points.ys.clear();
	computeAntiderivative<float>(points, function, 0.0, plotRect.X.Min, 200);
	flipY();
	ImPlot::PlotLine("antiderivative", points.xs.data(), points.ys.data(), points.xs.size());

	ImPlot::EndPlot();
}

void FirstOrderSystemGraph::bifurcationPlot(std::string_view parameterName) {
	if (!ImPlot::BeginPlot("##bifurcation plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		return;
	}
	if (plotCompiler.parameters.size() == 0 || !formulaInput.loopFunction.has_value()) {
		ImPlot::EndPlot();
		return;
	}

	const auto parameterIndex = plotCompiler.parameterNameToParameterIndex(parameterName);
	// TODO: Maybe delete the window if the parameter was deleted
	if (!parameterIndex.has_value()) {
		ASSERT_NOT_REACHED();
		ImPlot::EndPlot();
		return;
	}
	const auto& paramter = plotCompiler.parameters[*parameterIndex];

	ImPlot::SetupAxis(ImAxis_X1, paramter.name.c_str());
	ImPlot::SetupAxis(ImAxis_Y1, "x");

	const auto limits = ImPlot::GetPlotLimits();

	LoopFunctionArray input(plotCompiler.loopFunctionInputCount());
	LoopFunctionArray output(LOOP_FUNCTION_OUTPUT_COUNT);
	auto variablesBlock = plotCompiler.loopFunctionVariablesBlock;
	const auto steps = 200;
	for (i32 yi = 0; yi < steps; yi++) {
		for (i32 xi = 0; xi < steps; xi++) {
			const auto xt = float(xi) / float(steps - 1);
			const auto yt = float(yi) / float(steps - 1);
			const auto parameterValue = lerp(limits.X.Min, limits.X.Max, xt);
			const auto variableValue = lerp(limits.Y.Min, limits.Y.Max, yt);
			variablesBlock[0] = variableValue;
			variablesBlock[plotCompiler.parameterIndexToLoopFunctionVariableIndex(*parameterIndex)] = parameterValue;
			input.append(variablesBlock);
		}
	}
	output.resizeWithoutCopy(input.blockCount());

	(*formulaInput.loopFunction)(input, output);

	const auto grid = Span2d<const float>(output.data()->m256_f32, steps, steps);

	std::vector<Vec2> stableLines;
	std::vector<Vec2> unstableLines;

	std::vector<MarchingSquaresLine> marchingSquaresOutput;
	marchingSquares2(marchingSquaresOutput, grid, 0.0f, true);
	for (auto& segment : marchingSquaresOutput) {
		auto scale = [&](Vec2 pos) -> Vec2 {
			pos /= Vec2(grid.size());
			pos.x = lerp(limits.X.Min, limits.X.Max, pos.x);
			pos.y = lerp(limits.Y.Min, limits.Y.Max, pos.y);
			return pos;
		};
		const Vec2 a = scale(segment.a);
		const Vec2 b = scale(segment.b);
		/*
		Average
		float aboveValue = grid(segment.topLeftIndex().x, segment.topLeftIndex().y) + grid(segment.topRightIndex().x, segment.topRightIndex().y);
		float belowValue = grid(segment.bottomLeftIndex.x, segment.bottomLeftIndex.y) + grid(segment.bottomRightIndex().x, segment.bottomRightIndex().y);*/
		const float aboveValue = grid(segment.topLeftIndex().x, segment.topLeftIndex().y);
		const float belowValue = grid(segment.bottomLeftIndex.x, segment.bottomLeftIndex.y);
		if (aboveValue < belowValue) {
			stableLines.push_back(a);
			stableLines.push_back(b);
		} else {
			unstableLines.push_back(a);
			unstableLines.push_back(b);
		}
	}
	
	ImPlot::PushStyleColor(ImPlotCol_Line, STABLE_FIXED_POINTS_COLOR);
	plotVec2LineSegments("stable fixed points", stableLines);
	ImPlot::PopStyleColor();

	ImPlot::PushStyleColor(ImPlotCol_Line, UNSTABLE_FIXED_POINTS_COLOR);
	plotVec2LineSegments("unstable fixed points", unstableLines);
	ImPlot::PopStyleColor();

	ImPlot::EndPlot();
}

bool FirstOrderSystemGraph::examplesMenu() {
	if (ImGui::MenuItem("saddle node bifurcation")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.setFormulaInput(formulaInput, "a + x^2");
		return true;
	}
	if (ImGui::MenuItem("transcritical bifurcation")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.setFormulaInput(formulaInput, "ax - x^2");
		return true;
	}
	if (ImGui::MenuItem("pitchfork bifurcation")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.setFormulaInput(formulaInput, "ax - x^3");
		return true;
	}
	if (ImGui::MenuItem("pitchfork bifurcation with imperfection parameter")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.addParameterIfNotExists("b");
		plotCompiler.setFormulaInput(formulaInput, "ax - x^3 + b");
		return true;
	}
	if (ImGui::MenuItem("pitchfork bifurcation hysteresis")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.addParameterIfNotExists("b");
		plotCompiler.setFormulaInput(formulaInput, "ax + x^3 + -(1/10)x^5");
		// a is small the only fixed point is the center. Then as you increase the center becomes unstable and the points get attracted to the other fixed points, but if you vary the system back to below the bifurction point the points won't return back to the center fixed point. They only do after you vary it even further than the bifurcation point.
		// This is only visible nicly on the potential plot.
		return true;
	}

	return false;
}

void FirstOrderSystemGraph::settings() {
	plotCompiler.formulaInputGui("x'=", formulaInput);

	if (ImGui::Button("create bifurcation diagram")) {
		ImGui::OpenPopup(createBifurcationDiagramWindowName);
	}

	plotCompiler.settingsWindowContent();
	createBifurcationDiagramWindow();
}

void FirstOrderSystemGraph::createBifurcationDiagramWindow() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(createBifurcationDiagramWindowName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	if (selectedParameterIndex >= plotCompiler.parameters.size()) {
		selectedParameterIndex = 0;
	}

	const auto noParameters = plotCompiler.parameters.size() == 0;
	if (noParameters) {
		if (ImGui::BeginCombo("parameter", "no parameters to select")) {
			ImGui::EndCombo();
		}
	} else if (ImGui::BeginCombo("parameter", plotCompiler.parameters[selectedParameterIndex].name.c_str())) {
		for (int i = 0; i < plotCompiler.parameters.size(); i++) {

			const bool isSelected = (selectedParameterIndex == i);
			if (ImGui::Selectable(plotCompiler.parameters[i].name.c_str(), isSelected)) {
				selectedParameterIndex = i;
			}

			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}

		}
		ImGui::EndCombo();
	}

	if (!noParameters && ImGui::Button("create")) {
		bifurcationPlots.push_back(BifurcationPlot{
			.parameterName = plotCompiler.parameters[selectedParameterIndex].name
		});
		ImGui::CloseCurrentPopup();
		ImGui::SameLine();
	}

	if (ImGui::Button("cancel")) {
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
