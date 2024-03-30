#pragma once

#include <dependencies/math-compiler/src/runtime.hpp>
#include <dependencies/math-compiler/src/listIrCompilerMessageReporter.hpp>
#include <dependencies/math-compiler/src/listParserMessageReporter.hpp>
#include <dependencies/math-compiler/src/listScannerMessageReporter.hpp>
#include <dependencies/math-compiler/src/utils/stringStream.hpp>
#include <dependencies/math-compiler/src/glslCodeGenerator.hpp>
#include <StringRefStream.hpp>
#include <StringStream.hpp>
#include <list>
#include <ostream>

struct PlotCompiler {
	PlotCompiler();

	struct FormulaInput {
		static constexpr auto INPUT_MAX_SIZE = 256;

		char input[INPUT_MAX_SIZE] = "";
		std::string errorMessage;
		std::optional<Runtime::LoopFunction> loopFunction;
		bool modifiedThisFrame = false;
	};
	void formulaInputGui(const char* lhs, FormulaInput& formula);
	void setFormulaInput(FormulaInput& formula, std::string_view text);
private:
	void compileFormula(FormulaInput& formula);
public:
	bool tryCompileGlsl(std::ostream& out, const FormulaInput& input);

	std::vector<FormulaInput*> modifiedFormulaInputs;
	const std::vector<FormulaInput*>& updateEndOfFrame();

	// TODO: Make a container that can store items linearly without invalidating pointers. Simplest option would probably be an array with a bitset (they could be allocated together) that would track which slots are free.
	std::list<FormulaInput> allocatedFormulaInputs;
	FormulaInput* allocateFormulaInput();
	void freeFormulaInput(FormulaInput* formula);

	void recompileAllFormulas();

	void settingsWindowContent();

	const char* parameterSettingsWindowName = "parameter settings";
	void parameterSettingsWindow();
	// Using an index instead of a reference is better, because it prevents bugs from invalidated pointers.
	std::optional<i64> parameterSettingsWindowParameterIndex;

	const char* addParameterWindowName = "add parameter";
	void addParameterWindow();

	StringStream addParameterErrorMessage;
	bool addParameterError = false;

	std::vector<Variable> variables;
	void recalculateRuntimeVariables();
	bool variableExists(std::string_view name);

	struct Parameter {
		std::string name;
		float value;
		float valueMax;
		float valueMin;
	};

	std::vector<Parameter> parameters;
	bool parameterExists(std::string_view name);
	std::optional<i64> parameterNameToParameterIndex(std::string_view name) const;
	i64 parameterIndexToLoopFunctionVariableIndex(i64 parameterIndex) const;
	void addParameter(std::string_view name);
	void addParameterIfNotExists(std::string_view name);
	char parameterNameInput[256] = "";

	std::vector<float> loopFunctionVariablesBlock;
	i64 loopFunctionInputCount() const;
	float callLoopFunctionWithSingleOutput(const FormulaInput& formula, float f);

	ListScannerMessageReporter scannerReporter;
	ListParserMessageReporter parserReporter;
	ListIrCompilerMessageReporter irCompilerReporter;
	std::vector<Variable> runtimeVariables; // Can change. Should be recalculated before calling compile.
	Runtime runtime;
	GlslCodeGenerator glslCodeGenerator;
};