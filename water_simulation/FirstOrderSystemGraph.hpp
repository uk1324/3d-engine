#pragma once

#include <dependencies/math-compiler/src/runtime.hpp>
#include <dependencies/math-compiler/src/listIrCompilerMessageReporter.hpp>
#include <dependencies/math-compiler/src/listParserMessageReporter.hpp>
#include <dependencies/math-compiler/src/listScannerMessageReporter.hpp>
#include <dependencies/math-compiler/src/utils/stringStream.hpp>
#include <StringStream.hpp>

struct FirstOrderSystemGraph {
	FirstOrderSystemGraph();

	void update();
	void derivativePlot();
	void potentialPlot();
	void bifurcationPlot(std::string_view parameterName);
	void settings();
	static constexpr const char* infoWindowName = "info";
	void infoWindow();

	struct BifurcationPlot {
		std::string parameterName;
	};
	std::vector<BifurcationPlot> bifurcationPlots;

	void recompileFormula();
	void recalculateRuntimeVariables();

	const char* addParameterWindowName = "add parameter";
	void addParameterWindow();
	static constexpr auto parameterNameMaxSize = 256;
	char parameterNameInput[parameterNameMaxSize] = "";
	StringStream addParameterErrorMessage;
	bool addParameterError = false;

	const char* parameterSettingsWindowName = "parameter settings";
	void parameterSettingsWindow();
	// Using an index instead of a reference is better, because it prevents bugs from invalidated pointers.
	std::optional<i64> parameterIndex;

	LoopFunctionArray derivativePlotInput;
	LoopFunctionArray derivativePlotOutput;

	static constexpr i32 X_VARIABLE_INDEX = 0;
	i64 parameterIndexToLoopFunctionVariableIndex(i64 parameterIndex) const;
	static constexpr i64 LOOP_FUNCTION_OUTPUT_COUNT = 1;
	std::optional<Runtime::LoopFunction> loopFunction;
	std::vector<float> loopFunctionVariablesBlock;
	i64 loopFunctionInputCount() const;

	struct Parameter {
		std::string name;
		float value;
		float valueMax;
		float valueMin;
	};

	std::vector<Parameter> parameters;
	bool parameterExists(std::string_view name);
	std::optional<i64> parameterNameToParameterIndex(std::string_view name) const;

	static constexpr auto maxFormulaSize = 256;
	char formulaInput[maxFormulaSize] = "";
	std::vector<Variable> variables;
	bool variableExists(std::string_view name);

	StringStream errorMessageStream;
	ListScannerMessageReporter scannerReporter;
	ListParserMessageReporter parserReporter;
	ListIrCompilerMessageReporter irCompilerReporter;
	std::vector<Variable> runtimeVariables; // Can change. Should be recalculated before calling compile.
	Runtime runtime;
};