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
	void bifurcationPlot();
	void settings();

	//struct BifurctionPlot {
	//	std::string bifurcationPlotWindowName; // parameterName + " bifurcation diagram/plot"
	//	std::string parameterName;
	//};
	// std::vector<BifurcationPlot>

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
	i64 parameterIndexToLoopFunctionVariableIndex(i64 parameterIndex);
	std::optional<Runtime::LoopFunction> loopFunction;
	std::vector<float> loopFunctionVariablesBlock;

	struct Parameter {
		std::string name;
		float value;
		float valueMax;
		float valueMin;
	};

	std::vector<Parameter> parameters;
	bool parameterExists(std::string_view name);

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