#pragma once
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Mat2.hpp>
#include <vector>
#include <water_simulation/PlotCompiler.hpp>
#include <water_simulation/Eigenvectors.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <framework/Renderer2d.hpp>

struct SecondOrderSystemGraph {

	SecondOrderSystemGraph();

	void update(Renderer2d& renderer2d);
	void derivativePlot();
	std::vector<Vec2> fixedPoints;
	void plotStreamlines();
	void plotTestPoints();
	void plotFixedPoints();
	
	struct SampleVectorFieldState {
		std::vector<float> inputBlock;
		std::vector<__m256> input;
	} sampleVectorFieldState;
	Vec2 sampleVectorField(Vec2 v);

	void settings();
	bool examplesMenu();
	
	struct ComputeLoopFunctionOnVisibleRegionState {
		LoopFunctionArray input = LoopFunctionArray(0);
	};
	void computeLoopFunctionOnVisibleRegion(const Runtime::LoopFunction& function, LoopFunctionArray& output, i32 stepsX, i32 stepsY);
	ComputeLoopFunctionOnVisibleRegionState computeLoopFunctionOnVisibleRegionState;

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
	void calculateImplicitFunctionGraph(const Runtime::LoopFunction& function, std::vector<Vec2>& out);

	static void drawEigenvectors(Vec2 origin, const std::array<Eigenvector, 2>& eigenvectors, float scale, float complexPartTolerance);

	Mat2 calculateJacobian(Vec2 p);

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

	enum class ToolType {
		SPAWN_TEST_POINTS,
	};
	ToolType selectedToolType = ToolType::SPAWN_TEST_POINTS;
	const char* toolTypeNames = "spawn test points\0";

	struct LinearizationToolState {
		bool show = false;
		Vec2 pointToLinearlizeAbout = Vec2(0.0f);
		Mat2 jacobian = Mat2(Vec2(0.0f), Vec2(0.0f));

	} linearizationToolState;
	void linearizationToolSettings();
	void linearizationToolUpdate();

	struct BasinOfAttractionWindow {
		std::optional<ShaderProgram> shaderProgram;
		Camera camera;
		std::optional<Vec2> grabStartPosWorldSpace;

		void update(const SecondOrderSystemGraph& state, Renderer2d& renderer2d);
		void recompileShader(SecondOrderSystemGraph& state, Renderer2d& renderer2d);
	} basinOfAttractionWindow;

	Mat2 linearFormulaMatrix = Mat2(Vec2(0.0f), Vec2(0.0f));
	std::array<Eigenvector, 2> linearFormulaMatrixEigenvectors;

	PlotCompiler plotCompiler; // Has to be above FormulaInputs for thing to be initialized in the right order.
	PlotCompiler::FormulaInput& xFormulaInput;
	PlotCompiler::FormulaInput& yFormulaInput;
};
