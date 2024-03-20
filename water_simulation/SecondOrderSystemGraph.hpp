#pragma once
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <vector>
#include <water_simulation/PlotCompiler.hpp>

struct SecondOrderSystemGraph {
	SecondOrderSystemGraph();

	void update();
	void derivativePlot();
	void settings();
	bool examplesMenu();

	//std::vector<Vec2> points;

	bool paused = false;
	struct TestPoint {
		Vec2 pos;
		std::vector<Vec2> history;
	};
	std::vector<TestPoint> testPoints;

	float spacing = 0.1f;

	PlotCompiler::FormulaInput xFormulaInput;
	PlotCompiler::FormulaInput yFormulaInput;
	PlotCompiler plotCompiler;
};
