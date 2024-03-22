#pragma once

#include <water_simulation/PlotCompiler.hpp>

struct FirstOrderSystemGraph {
	FirstOrderSystemGraph();

	void update();
	void derivativePlot();
	void potentialPlot();
	void bifurcationPlot(std::string_view parameterName);
	bool examplesMenu();
	void settings();

	static constexpr const char* createBifurcationDiagramWindowName = "bifurcation diagram";
	void createBifurcationDiagramWindow();
	i64 selectedParameterIndex = 0;

	struct BifurcationPlot {
		std::string parameterName;
	};
	std::vector<BifurcationPlot> bifurcationPlots;

	LoopFunctionArray derivativePlotInput;
	LoopFunctionArray derivativePlotOutput;

	static constexpr i32 X_VARIABLE_INDEX = 0;
	static constexpr i64 LOOP_FUNCTION_OUTPUT_COUNT = 1;

	PlotCompiler plotCompiler; // Has to be above FormulaInputs for thing to be initialized in the right order.
	PlotCompiler::FormulaInput& formulaInput;
};