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
	void plotStreamlines();
	void plotTestPoints();
	void settings();
	bool examplesMenu();

	struct ImplicitFunctionGraph {
		// Can't be a reference to be (copyable?) I think not sure.
		PlotCompiler::FormulaInput* formulaInput;
		Vec3 color;
		bool isHidden = false;
	};
	std::vector<ImplicitFunctionGraph> implicitFunctionGraphs;

	bool implicitFunctionGraphSettings(ImplicitFunctionGraph& graph);
	void drawImplicitFunctionGraph(
		const char* label, 
		Vec3 color,
		const PlotCompiler::FormulaInput& formula);

	bool spawnPointsOnBoundaryNextFrame = false;
	bool spawnGridOfPointsNextFrame = false;

	bool paused = false;
	struct TestPoint {
		// TODO: Maybe add a way to integrate backwards and forwards in time.
		Vec2 pos;
		std::vector<Vec2> history;
	};
	std::vector<TestPoint> testPoints;

	bool drawNullclines = false;

	float spacing = 0.2f;

	enum class FormulaType {
		LINEAR,
		GENERAL
	};
	FormulaType formulaType = FormulaType::GENERAL;
	static constexpr const char* formulaTypeNames = "linear\0general\0";

	Mat2 linearFormulaMatrix = Mat2(Vec2(0.0f), Vec2(0.0f));
	std::array<Eigenvector, 2> linearFormulaMatrixEigenvectors;

	PlotCompiler plotCompiler; // Has to be above FormulaInputs for thing to be initialized in the right order.
	PlotCompiler::FormulaInput& xFormulaInput;
	PlotCompiler::FormulaInput& yFormulaInput;

	std::vector<Vec2> areaParticles;
};
