#include <water_simulation/FirstOrderSystemGraph.hpp>
#include <dependencies/math-compiler/src/errorMessage.hpp>
#include <imgui/implot.h>
#include <StringUtils.hpp>
#include <engine/Math/Utils.hpp>
#include <Gui.hpp>
#include <algorithm>
#include <water_simulation/assets/IconsFontAwesome5.h>
#include <imgui/imgui_internal.h>

FirstOrderSystemGraph::FirstOrderSystemGraph() 
	: runtime(scannerReporter, parserReporter, irCompilerReporter)
	, derivativePlotInput(1)
	, derivativePlotOutput(1) {

	variables.push_back(Variable{ .name = "x" });

	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	//io.Fonts->Fonts
	float baseFontSize = 20.0f; // 13.0f is the size of the default font. Change to the font size you use.
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	// merge in icons from Font Awesome
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

		//DockBuilderDockWindow("plot", rightId);
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

	//ImPlot::ShowDemoWindow();
	//ImGui::ShowDemoWindow();

	if (loopFunction.has_value()) {
		const auto limits = ImPlot::GetPlotLimits();
		//ImPlot::axis
		const auto min = float(limits.X.Min);
		const auto max = float(limits.X.Max);
		i32 steps = 200;
		derivativePlotInput.reset(variables.size() + parameters.size());
		std::vector<float> block;
		for (i32 i = 0; i <= steps - 1; i++) {
			const auto t = float(i) / float(steps - 1);
			const auto x = lerp(min, max, t);
			block.clear();
			block.push_back(x);
			for (const auto& parameter : parameters) {
				block.push_back(parameter.value);
			}
			derivativePlotInput.append(std::span<const float>(block));
		}
		derivativePlotOutput.resizeWithoutCopy(derivativePlotInput.blockCount);
		i64 dataCount = derivativePlotInput.blockCount / LoopFunctionArray::ITEMS_PER_DATA;
		if (derivativePlotInput.blockCount % LoopFunctionArray::ITEMS_PER_DATA != 0) {
			dataCount += 1;
		}
		(*loopFunction)(derivativePlotInput.data, derivativePlotOutput.data, dataCount);
		//memset(derivativePlotOutput.data, 0, derivativePlotOutput.dataCapacity * sizeof(__m256));
		std::vector<float> xs;
		std::vector<float> ys;
		for (i64 i = 0; i < derivativePlotOutput.blockCount; i++) {
			const float x = derivativePlotInput(i, 0);
			const float y = derivativePlotOutput(i, 0);
			xs.push_back(x);
			ys.push_back(y);
		}
		ImPlot::PlotLine("derivative plot", xs.data(), ys.data(), xs.size());
	}

	ImPlot::EndPlot();
}

void FirstOrderSystemGraph::potentialPlot() {
	if (!ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		return;
	}
	ImPlot::SetupAxis(ImAxis_X1, "x");
	ImPlot::SetupAxis(ImAxis_Y1, "potential");

	ImPlot::EndPlot();
}

void FirstOrderSystemGraph::settings() {
	bool recompile = false;

	/*auto editor = [&]() {
		if (Gui::inputText("x' = ", formulaInput, std::size(formulaInput))) {
			recompile = true;
		}
	};
	GUI_PROPERTY_EDITOR({ editor(); });*/

	ImGui::Text("x'=");
	ImGui::SameLine();
	if (ImGui::InputText("##formulaInput", formulaInput, std::size(formulaInput))) {
		recompile = true;
	}

	if (recompile) {
		recompileFormula();
	}

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
	for (const auto& variable : variables) {
		runtimeVariables.push_back(variable);
	}
	for (const auto& parameter : parameters) {
		runtimeVariables.push_back(Variable{ .name = parameter.name });
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

		if (variableExists(parameterName)) {
			addParameterErrorMessage.string().clear();
			putnn(addParameterErrorMessage, "'%' is already a variable name", parameterName);
			addParameterError = true;
		} else if (parameterExists(parameterName)) {
			addParameterErrorMessage.string().clear();
			putnn(addParameterErrorMessage, "'%' is already a parameter name", parameterName);
			addParameterError = true;
		} else {
			parameters.push_back(Parameter{
				.name = std::string(parameterName),
				.value = 0.0f,
				.valueMax = 10.0f,
				.valueMin = -10.0f
			});
			recompileFormula();
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

	if (ImGui::Button("close")) {
		parameterIndex = std::nullopt;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

bool FirstOrderSystemGraph::parameterExists(std::string_view name) {
	return std::find_if(parameters.begin(), parameters.end(), [&](const Parameter& p) { return p.name == name; }) != parameters.end();
}

bool FirstOrderSystemGraph::variableExists(std::string_view name) {
	return std::find_if(variables.begin(), variables.end(), [&](const Variable& v) { return v.name == name; }) != variables.end();
}
