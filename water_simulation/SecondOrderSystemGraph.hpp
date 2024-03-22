#pragma once
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Mat2.hpp>
#include <vector>
#include <water_simulation/PlotCompiler.hpp>
#include <water_simulation/Eigenvectors.hpp>

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

	float spacing = 0.2f;

	enum class FormulaType {
		LINEAR,
		GENERAL
	};
	FormulaType formulaType = FormulaType::GENERAL;
	static constexpr const char* formulaTypeNames = "linear\0general\0";

	Mat2 linearFormulaMatrix = Mat2(Vec2(0.0f), Vec2(0.0f));
	std::array<Eigenvector, 2> linearFormulaMatrixEigenvectors;

	PlotCompiler::FormulaInput xFormulaInput;
	PlotCompiler::FormulaInput yFormulaInput;
	PlotCompiler plotCompiler;
};
