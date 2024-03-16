#include "PlotCompiler.hpp"
#include <imgui/implot.h>
#include <StringUtils.hpp>
#include <utils/put.hpp>
#include <dependencies/math-compiler/src/errorMessage.hpp>
#include <water_simulation/assets/IconsFontAwesome5.h>

PlotCompiler::PlotCompiler()
	: runtime(scannerReporter, parserReporter, irCompilerReporter) {

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

void PlotCompiler::recompileAllFormulas() {
	for (auto formula : formulaInputs) {
		compileFormula(*formula);
	}
}

void PlotCompiler::settingsWindowContent() {

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
			parameterSettingsWindowParameterIndex = i;
			ImGui::OpenPopup(parameterSettingsWindowName);
		}

		ImGui::PopStyleColor();
		ImGui::PopID();
	}

	const auto parameterIndexHasValue = parameterSettingsWindowParameterIndex.has_value();
	if (parameterIndexHasValue) ImGui::PushID(*parameterSettingsWindowParameterIndex);
	parameterSettingsWindow();
	if (parameterIndexHasValue) ImGui::PopID();
}

void PlotCompiler::parameterSettingsWindow() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(parameterSettingsWindowName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	if (!parameterSettingsWindowParameterIndex.has_value()) {
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return;
	}

	if (*parameterSettingsWindowParameterIndex >= parameters.size()) {
		ASSERT_NOT_REACHED();
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return;
	}

	auto& parameter = parameters[*parameterSettingsWindowParameterIndex];
	ImGui::InputFloat("min", &parameter.valueMin);
	ImGui::InputFloat("max", &parameter.valueMax);

	if (ImGui::Button("close")) {
		parameterSettingsWindowParameterIndex = std::nullopt;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void PlotCompiler::addParameterWindow() {
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
			// do nothing
		} else if (variableExists(parameterName)) {
			addParameterErrorMessage.string().clear();
			putnn(addParameterErrorMessage, "'%' is already a variable name", parameterName);
			addParameterError = true;
		}
		else if (parameterExists(parameterName)) {
			addParameterErrorMessage.string().clear();
			putnn(addParameterErrorMessage, "'%' is already a parameter name", parameterName);
			addParameterError = true;
		}
		else {
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

void PlotCompiler::formulaInputGui(const char* lhs, FormulaInput& formula) {
	ImGui::Text(lhs);
	ImGui::SameLine();
	bool recompile = false;
	if (ImGui::InputText("##formulaInput", formula.input, FormulaInput::INPUT_MAX_SIZE)) {
		recompile = true;
	}

	if (recompile) {
		compileFormula(formula);
	}

	// TODO: Could add an message to add a paramter if there is an error about an undefined variable.
	const auto scannerError = scannerReporter.errors.size() != 0;
	const auto parserError = parserReporter.errors.size() != 0;
	const auto irCompilerError = irCompilerReporter.errors.size() != 0;
	const auto anyError = scannerError || parserError || irCompilerError;
	if (trimString(formula.input) != "" && anyError) {
		formula.errorMessageStream.string().clear();
		if (scannerError) {
			outputScannerErrorMessage(formula.errorMessageStream, scannerReporter.errors[0], formula.input, false);
		}
		else if (parserError) {
			outputParserErrorMessage(formula.errorMessageStream, parserReporter.errors[0], formula.input, false);
		}
		else if (irCompilerError) {
			outputIrCompilerErrorMessage(formula.errorMessageStream, irCompilerReporter.errors[0], formula.input, false);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Text("%s", formula.errorMessageStream.string().c_str());
		ImGui::PopStyleColor();
	}
}

void PlotCompiler::setFormulaInput(FormulaInput& formula, std::string_view text) {
	// +1 because of the null byte.
	if (text.length() + 1 > FormulaInput::INPUT_MAX_SIZE) {
		ASSERT_NOT_REACHED();
		return;
	}
	memcpy(formula.input, text.data(), text.size());
	formula.input[text.size()] = '\0';
	compileFormula(formula);
}

void PlotCompiler::compileFormula(FormulaInput& formula) {
	scannerReporter.reset();
	parserReporter.reset();
	irCompilerReporter.reset();

	put("compiling '%'", formula.input);

	recalculateRuntimeVariables();
	formula.loopFunction = runtime.compileFunction(formula.input, runtimeVariables);
}


void PlotCompiler::recalculateRuntimeVariables() {
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

void PlotCompiler::addParameter(std::string_view name) {
	parameters.push_back(Parameter{
		.name = std::string(name),
		.value = 0.0f,
		.valueMax = 10.0f,
		.valueMin = -10.0f
	});
	recompileAllFormulas();
}

i64 PlotCompiler::parameterIndexToLoopFunctionVariableIndex(i64 parameterIndex) const {
	static constexpr auto variableCount = 1;
	return variableCount + parameterIndex;
}

bool PlotCompiler::parameterExists(std::string_view name) {
	return std::ranges::find_if(parameters, [&](const Parameter& p) { return p.name == name; }) != parameters.end();
}

i64 PlotCompiler::loopFunctionInputCount() const {
	return 1 + parameters.size();
}

float PlotCompiler::callLoopFunctionWithSingleOutput(const FormulaInput& formula, float f) {
	std::vector<__m256> vs;
	const auto x = _mm256_set1_ps(f);
	vs.push_back(x);
	for (int i = 0; i < parameters.size(); i++) {
		const auto a = _mm256_set1_ps(parameters[i].value);
		vs.push_back(a);
	}
	__m256 out;
	(*formula.loopFunction)(vs.data(), &out, 1);
	return out.m256_f32[0];
}

bool PlotCompiler::variableExists(std::string_view name) {
	return std::ranges::find_if(variables, [&](const Variable& v) { return v.name == name; }) != variables.end();
}

std::optional<i64> PlotCompiler::parameterNameToParameterIndex(std::string_view name) const {
	const auto it = std::ranges::find_if(parameters, [&](const Parameter& p) { return p.name == name; });
	return it - parameters.begin();
}