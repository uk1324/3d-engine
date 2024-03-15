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
#include <engine/Math/RootFinding/bisection.hpp>
#include <engine/Math/MarchingSquares.hpp>

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
	: runtime(scannerReporter, parserReporter, irCompilerReporter)
	, derivativePlotInput(1)
	, derivativePlotOutput(1) {

	variables.push_back(Variable{ .name = "x" });

	ImGuiIO& io = ImGui::GetIO();
	float baseFontSize = 20.0f;
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	io.Fonts->AddFontFromFileTTF("water_simulation/assets/" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges);
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

	if (!loopFunction.has_value()) {
		ImPlot::EndPlot();
		return;
	}

	const auto limits = ImPlot::GetPlotLimits();
	const auto min = float(limits.X.Min);
	const auto max = float(limits.X.Max);
	i32 steps = 200;
	derivativePlotInput.reset(variables.size() + parameters.size());
	for (i32 i = 0; i <= steps - 1; i++) {
		const auto t = float(i) / float(steps - 1);
		const auto x = lerp(min, max, t);
		loopFunctionVariablesBlock[0] = x;
		derivativePlotInput.append(loopFunctionVariablesBlock);
	}
	derivativePlotOutput.resizeWithoutCopy(derivativePlotInput.blockCount_);
	(*loopFunction)(derivativePlotInput, derivativePlotOutput);

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
			[this](float x) { return callLoopFunctionWithSingleOutput(x); },
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

	const auto green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImPlot::PushStyleColor(ImPlotCol_MarkerFill, green);
	ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, green);
	scatterPlotVec2s("stable fixed points", stableFixedPoints);
	ImPlot::PopStyleColor();
	ImPlot::PopStyleColor();

	const auto red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImPlot::PushStyleColor(ImPlotCol_MarkerFill, red);
	ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, red);
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

	if (!loopFunction.has_value()) {
		ImPlot::EndPlot();
		return;
	}

	const auto plotRect = ImPlot::GetPlotLimits();
	Points2<float> points;
	auto function = [&](float x) {
		return callLoopFunctionWithSingleOutput(x);
	};

	computeAntiderivative<float>(points, function, 0.0, plotRect.X.Max, 200);
	ImPlot::PlotLine("antiderivative", points.xs.data(), points.ys.data(), points.xs.size());
	points.xs.clear();
	points.ys.clear();
	computeAntiderivative<float>(points, function, 0.0, plotRect.X.Min, 200);
	ImPlot::PlotLine("antiderivative", points.xs.data(), points.ys.data(), points.xs.size());

	ImPlot::EndPlot();
}

void FirstOrderSystemGraph::bifurcationPlot(std::string_view parameterName) {
	if (!ImPlot::BeginPlot("##bifurcation plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		return;
	}
	if (parameters.size() == 0 || !loopFunction.has_value()) {
		ImPlot::EndPlot();
		return;
	}

	const auto parameterIndex = parameterNameToParameterIndex(parameterName);
	if (!parameterIndex.has_value()) {
		ASSERT_NOT_REACHED();
		ImPlot::EndPlot();
		return;
	}
	const auto& paramter = parameters[*parameterIndex];

	ImPlot::SetupAxis(ImAxis_X1, paramter.name.c_str());
	ImPlot::SetupAxis(ImAxis_Y1, "x");

	const auto limits = ImPlot::GetPlotLimits();

	LoopFunctionArray input(loopFunctionInputCount());
	LoopFunctionArray output(LOOP_FUNCTION_OUTPUT_COUNT);
	auto variablesBlock = loopFunctionVariablesBlock;
	const auto steps = 200;
	for (i32 yi = 0; yi < steps; yi++) {
		for (i32 xi = 0; xi < steps; xi++) {
			const auto xt = float(xi) / float(steps - 1);
			const auto yt = float(yi) / float(steps - 1);
			const auto parameterValue = lerp(limits.X.Min, limits.X.Max, xt);
			const auto variableValue = lerp(limits.Y.Min, limits.Y.Max, yt);
			variablesBlock[0] = variableValue;
			variablesBlock[parameterIndexToLoopFunctionVariableIndex(*parameterIndex)] = parameterValue;
			input.append(variablesBlock);
		}
	}
	output.resizeWithoutCopy(input.blockCount());

	(*loopFunction)(input, output);

	const auto grid = Span2d<const float>(output.data()->m256_f32, steps, steps);

	std::vector<MarchingSquaresLine> marchingSquaresOutput;
	marchingSquares2(marchingSquaresOutput, grid, 0.0f, true);
	for (auto& segment : marchingSquaresOutput) {
		auto scale = [&](Vec2 pos) -> Vec2 {
			pos /= Vec2(grid.size());
			pos.x = lerp(limits.X.Min, limits.X.Max, pos.x);
			pos.y = lerp(limits.Y.Min, limits.Y.Max, pos.y);
			return pos;
		};
		segment.a = scale(segment.a);
		segment.b = scale(segment.b);
	}
	
	const auto pointsData = reinterpret_cast<const float*>(marchingSquaresOutput.data());
	ImPlot::PlotLine("fixed points", pointsData, pointsData + 1, marchingSquaresOutput.size() * 2, ImPlotLineFlags_Segments, 0, sizeof(float) * 2);

	ImPlot::EndPlot();
}

void FirstOrderSystemGraph::settings() {
	ImGui::Text("x'=");
	ImGui::SameLine();
	bool recompile = false;
	if (ImGui::InputText("##formulaInput", formulaInput, std::size(formulaInput))) {
		recompile = true;
	}

	if (recompile) {
		recompileFormula();
	}

	// TODO: Could add an message to add a paramter if there is an error about an undefined variable.
	const auto scannerError = scannerReporter.errors.size() != 0;
	const auto parserError = parserReporter.errors.size() != 0;
	const auto irCompilerError = irCompilerReporter.errors.size() != 0;
	const auto anyError = scannerError || parserError || irCompilerError;
	if (trimString(formulaInput) != "" && anyError) {
		errorMessageStream.string().clear();
		if (scannerError) {
			outputScannerErrorMessage(errorMessageStream, scannerReporter.errors[0], formulaInput, false);
		} else if (parserError) {
			outputParserErrorMessage(errorMessageStream, parserReporter.errors[0], formulaInput, false);
		} else if (irCompilerError) {
			outputIrCompilerErrorMessage(errorMessageStream, irCompilerReporter.errors[0], formulaInput, false);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Text("%s", errorMessageStream.string().c_str());
		ImGui::PopStyleColor();
	}

	if (ImGui::Button("add parameter")) {
		ImGui::OpenPopup(addParameterWindowName);
	}
	addParameterWindow();


	for (int i = 0; i < parameters.size(); i++) {
		auto& parameter = parameters[i];
		ImGui::PushID(i);
		ImGui::Text("%s", parameter.name.c_str());
		ImGui::SameLine();
		const auto disableSlider = parameter.valueMin >= parameter.valueMax;
		if (disableSlider) ImGui::BeginDisabled();
		ImGui::SliderFloat("##parameterSlider", &parameter.value, parameter.valueMin, parameter.valueMax);
		if (disableSlider) ImGui::EndDisabled();

		loopFunctionVariablesBlock[parameterIndexToLoopFunctionVariableIndex(i)] = parameter.value;

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		if (ImGui::Button(ICON_FA_COG)) {
			parameterIndex = i;
			ImGui::OpenPopup(parameterSettingsWindowName);
		}
		
		ImGui::PopStyleColor();
		ImGui::PopID();
	}

	const auto parameterIndexHasValue = parameterIndex.has_value();
	if (parameterIndexHasValue) ImGui::PushID(*parameterIndex);
	parameterSettingsWindow();
	if (parameterIndexHasValue) ImGui::PopID();
}

bool FirstOrderSystemGraph::examplesMenu() {
	auto addParameterIfNotExists = [this]() {
		const auto name = "a";
		if (!parameterExists(name)) {
			addParameter(name);
		}
	};

	auto setFormula = [this](std::string_view text) {
		if (text.length() + 1 > maxFormulaSize) {
			ASSERT_NOT_REACHED();
			return;
		}
		memcpy(formulaInput, text.data(), text.size());
		formulaInput[text.size()] = '\0';
		recompileFormula();
	};

	if (ImGui::MenuItem("saddle node bifurcation")) {
		addParameterIfNotExists();
		setFormula("a + x^2");
		return true;
	}
	if (ImGui::MenuItem("transcritical bifurcation")) {
		setFormula("ax - x^2");
		addParameterIfNotExists();
		return true;
	}
	if (ImGui::MenuItem("pitchfork bifurcation")) {
		addParameterIfNotExists();
		setFormula("ax - x^3");
		return true;
	}

	return false;
}

void FirstOrderSystemGraph::recompileFormula() {
	scannerReporter.reset();
	parserReporter.reset();
	irCompilerReporter.reset();

	put("recompiling");

	recalculateRuntimeVariables();
	loopFunction = runtime.compileFunction(formulaInput, runtimeVariables);
}

void FirstOrderSystemGraph::recalculateRuntimeVariables() {
	runtimeVariables.clear();

	loopFunctionVariablesBlock.clear();

	for (const auto& variable : variables) {
		runtimeVariables.push_back(variable);
		loopFunctionVariablesBlock.push_back(0.0f);
	}
	for (const auto& parameter : parameters) {
		runtimeVariables.push_back(Variable{ .name = parameter.name });
		loopFunctionVariablesBlock.push_back(parameter.value);
	}
}

void FirstOrderSystemGraph::addParameterWindow() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(addParameterWindowName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	if (ImGui::InputText("##parameterName", parameterNameInput, std::size(parameterNameInput))) {
		// If there was an error an the name was changed the don't show the error anymore.
		addParameterError = false;
	}

	if (addParameterError == true) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Text("%s", addParameterErrorMessage.string().c_str());
		ImGui::PopStyleColor();
	}

	if (ImGui::Button("add")) {
		const std::string_view parameterName = parameterNameInput;

		if (trimString(parameterName).length() == 0) {

		} else if (variableExists(parameterName)) {
			addParameterErrorMessage.string().clear();
			putnn(addParameterErrorMessage, "'%' is already a variable name", parameterName);
			addParameterError = true;
		} else if (parameterExists(parameterName)) {
			addParameterErrorMessage.string().clear();
			putnn(addParameterErrorMessage, "'%' is already a parameter name", parameterName);
			addParameterError = true;
		} else {
			addParameter(parameterName);
			parameterNameInput[0] = '\0';
			ImGui::CloseCurrentPopup();
			addParameterError = false;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("cancel")) {
		parameterNameInput[0] = '\0';
		addParameterError = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void FirstOrderSystemGraph::addParameter(std::string_view name) {
	parameters.push_back(Parameter{
		.name = std::string(name),
		.value = 0.0f,
		.valueMax = 10.0f,
		.valueMin = -10.0f
	});
	recompileFormula();
}

void FirstOrderSystemGraph::parameterSettingsWindow() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(parameterSettingsWindowName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	if (!parameterIndex.has_value()) {
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return;
	}

	if (*parameterIndex >= parameters.size()) {
		ASSERT_NOT_REACHED();
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return;
	}

	auto& parameter = parameters[*parameterIndex];
	ImGui::InputFloat("min", &parameter.valueMin);
	ImGui::InputFloat("max", &parameter.valueMax);

	if (ImGui::Button("create bifurcation diagram")) {
		bifurcationPlots.push_back(BifurcationPlot{
			.parameterName = parameter.name
		});
		ImGui::CloseCurrentPopup();
	}

	if (ImGui::Button("close")) {
		parameterIndex = std::nullopt;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

i64 FirstOrderSystemGraph::parameterIndexToLoopFunctionVariableIndex(i64 parameterIndex) const {
	static constexpr auto variableCount = 1;
	return variableCount + parameterIndex;
}

bool FirstOrderSystemGraph::parameterExists(std::string_view name) {
	return std::ranges::find_if(parameters, [&](const Parameter& p) { return p.name == name; }) != parameters.end();
}

i64 FirstOrderSystemGraph::loopFunctionInputCount() const {
	return 1 + parameters.size();
}

float FirstOrderSystemGraph::callLoopFunctionWithSingleOutput(float f) {
	std::vector<__m256> vs;
	const auto x = _mm256_set1_ps(f);
	vs.push_back(x);
	for (int i = 0; i < parameters.size(); i++) {
		const auto a = _mm256_set1_ps(parameters[i].value);
		vs.push_back(a);
	}
	__m256 out;
	(*loopFunction)(vs.data(), &out, 1);
	return out.m256_f32[0];
}

bool FirstOrderSystemGraph::variableExists(std::string_view name) {
	return std::ranges::find_if(variables, [&](const Variable& v) { return v.name == name; }) != variables.end();
}

std::optional<i64> FirstOrderSystemGraph::parameterNameToParameterIndex(std::string_view name) const {
	const auto it = std::ranges::find_if(parameters, [&](const Parameter& p) { return p.name == name; });
	return it - parameters.begin();
}
