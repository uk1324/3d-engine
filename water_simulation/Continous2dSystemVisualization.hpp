#pragma once
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Mat2.hpp>
#include <vector>
#include <water_simulation/PlotCompiler.hpp>
#include <water_simulation/Eigenvectors.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <water_simulation/RenderWindow.hpp>
#include <framework/Renderer2d.hpp>

struct Continous2dSystemVisualization {
	enum class FormulaType {
		CARTESIAN_LINEAR,
		CARTESIAN,
		POLAR
	};
	static constexpr const char* formulaTypeNames = "cartesian linear\0cartesian\0polar\0";

	Continous2dSystemVisualization();

	void update(Renderer2d& renderer2d);
	void derivativePlot(Renderer2d& renderer2d);
	std::vector<Vec2> fixedPoints;

	struct StreamlineSettings {
		float spacing = 0.2f;
	} streamlineSettings;
	static void plotStreamlines(
		const PlotCompiler& plotCompiler, 
		const Runtime::LoopFunction& f0, 
		const Runtime::LoopFunction& f1,
		FormulaType formulaType,
		const StreamlineSettings& settings);
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
	static void linearizationToolUpdate(LinearizationToolState& s, const std::vector<Vec2> fixedPoints);

	struct BasinOfAttractionState {
		bool show = false;
		std::optional<ShaderProgram> shaderProgram;
		float resolutionScale = 0.1f;
		float opacity = 0.5f;
		int iterations = 100;
		RenderWindow2d renderWindow;
		static constexpr int MAX_FIXED_POINT_COUNT = 15;

		void render(
			const Continous2dSystemVisualization& state, 
			const Aabb& view,
			Renderer2d& renderer2d);
		void recompileShader(Continous2dSystemVisualization& state, Renderer2d& renderer2d);
		void settings();
	} basinOfAttractionState;

	void changeFormulaTypeToCartesian();
	void changeFormulaTypeToCartesianLinear();
	void changeFormulaTypeToPolar();

	void updateLinearFormula();

	Mat2 linearFormulaMatrix = Mat2(Vec2(0.0f), Vec2(0.0f));
	std::array<Eigenvector, 2> linearFormulaMatrixEigenvectors;

	// Could either have different inputs for different input types (cartesian/polar) or the same inputs.
	PlotCompiler plotCompiler; // Has to be above FormulaInputs for thing to be initialized in the right order.
	PlotCompiler::FormulaInput& formulaInput0;
	PlotCompiler::FormulaInput& formulaInput1;
	/*
	Formula inputs interpreation
	(0, 1) ->
	CARTESIAN (x, y)
	POLAR (a, r)
	*/

	FormulaType formulaType = FormulaType::CARTESIAN;
	bool formulaTypeIsCartesian() const;

	std::vector<Vec2> areaParticles;
};
