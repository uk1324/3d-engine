#pragma once

struct FirstOrderSystemGraph {
	FirstOrderSystemGraph();

	void update();
	void plot();
	void plotSettings();

	static constexpr auto maxFormulaSize = 256;

	char formulaInput[maxFormulaSize] = "";
};