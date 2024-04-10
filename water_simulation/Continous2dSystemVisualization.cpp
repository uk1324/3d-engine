#include <water_simulation/Continous2dSystemVisualization.hpp>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/Color.hpp>
#include <water_simulation/PlotUtils.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/MarchingSquares.hpp>
#include <engine/Math/Utils.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/LineSegment.hpp>
#include <Gui.hpp>
#include <Array2d.hpp>
#include <iomanip>

// TODO add an input option for x''= as a function of x and x'. just replace the fields. Add a button to show the energy graph. The graph the function the force has to be integrated. For it to be conservative it can't depend on x'. Could also just graph the potential (could draw the fixed points on the graph).

// TODO: Could graph the potentials of conservative/irrotational fields.

// TODO: Could allow intersecting a sufrace with a plane. The result would just be the sum of the intersection with each triangle.

// TODO: To find the fixed points could compute the abs() of all the values then find the local minima. If the minima are near zero then I could run root finding
 
// Could add functions to plot compiler. aliasVariable(i64 index, std::string name) this would rename the variable and if needed remove all the parameters that have the same name (could open a window asking if this should happen). And disableVariable(i64 index), which would rename it to "". Could have a flag in Variable that the parser checks.
// A second order system can always be interpreted as some system with energy and forcing. Allow drawing the energy surface for any system x'' = f(x, x').

/* 
Polar to cartesian
x = rcos(a)
dx/dt = dr/dt * cos(a) - sin(a) da/dt * r =
dx/dt = dr/dt * x / r - y da/dt

y = rsin(a)
dy/dt = dr/dt * sin(a) + cos(a) da/dt * r =
dy/dt = dr/dt * y / r + x da/dt
*/

Vec2 polarDerivativeToCartesianDerivative(float dadt, float drdt, Vec2 cartesianPos, float r) {
	const auto dxdt = drdt * cartesianPos.x / r - cartesianPos.y * dadt;
	const auto dydt = drdt * cartesianPos.y / r + cartesianPos.x * dadt;
	return Vec2(dxdt, dydt);
}

static constexpr auto VARIABLE_0_INDEX_IN_BLOCK = 0;
static constexpr auto VARIABLE_1_INDEX_IN_BLOCK = 1;

static constexpr auto VARIABLE_X_INDEX_IN_BLOCK = VARIABLE_0_INDEX_IN_BLOCK;
static constexpr auto VARIABLE_Y_INDEX_IN_BLOCK = VARIABLE_1_INDEX_IN_BLOCK;

static constexpr auto VARIABLE_A_INDEX_IN_BLOCK = VARIABLE_0_INDEX_IN_BLOCK;
static constexpr auto VARIABLE_R_INDEX_IN_BLOCK = VARIABLE_1_INDEX_IN_BLOCK;

Continous2dSystemVisualization::Continous2dSystemVisualization()
	: formulaInput0(*plotCompiler.allocateFormulaInput())
	, formulaInput1(*plotCompiler.allocateFormulaInput()) {

	plotCompiler.variables.push_back(Variable{ .name = "x" });
	plotCompiler.variables.push_back(Variable{ .name = "y" });
}

void Continous2dSystemVisualization::update(Renderer2d& renderer2d) {
	auto id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	const auto derivativePlotWindowName = "derivative plot";
	const auto settingsWindowName = "settings";

	static bool firstFrame = true;
	if (firstFrame) {
		ImGui::DockBuilderRemoveNode(id);
		ImGui::DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

		ImGui::DockBuilderDockWindow(derivativePlotWindowName, rightId);
		ImGui::DockBuilderDockWindow(settingsWindowName, leftId);

		ImGui::DockBuilderFinish(id);
		firstFrame = false;
	}

	ImGui::Begin(derivativePlotWindowName);
	derivativePlot();
	ImGui::End();

	ImGui::Begin(settingsWindowName);
	settings();
	ImGui::End();

	const auto& modifiedFormulaInputs = plotCompiler.updateEndOfFrame();

	//for (const auto& input : modifiedFormulaInputs) {
	//	if (input == &xFormulaInput || input == &yFormulaInput) {
	//		basinOfAttractionWindow.recompileShader(*this, renderer2d);
	//	}
	//}
	//basinOfAttractionWindow.update(*this, renderer2d);
}

void Continous2dSystemVisualization::derivativePlot() {
	if (!ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		return;
	}

	ImPlot::SetupAxis(ImAxis_X1, plotCompiler.variables[VARIABLE_0_INDEX_IN_BLOCK].name.data());
	ImPlot::SetupAxis(ImAxis_Y1, plotCompiler.variables[VARIABLE_1_INDEX_IN_BLOCK].name.data());

	const auto grey = Vec4{ 0.5f, 0.5f, 0.5f, 1.0f };
	ImPlot::PushStyleColor(ImPlotCol_PlotBg, grey);

	if (formulaInput0.loopFunction.has_value() && formulaInput1.loopFunction.has_value()) {
		plotStreamlines(plotCompiler, *formulaInput0.loopFunction, *formulaInput1.loopFunction, formulaType, streamlineSettings);
	}

	const auto isFormulaCartesian = formulaType == FormulaType::CARTESIAN || formulaType == FormulaType::CARTESIAN_LINEAR;

	if (isFormulaCartesian) {
		plotFixedPoints();
	}

	plotTestPoints();

	if (formulaType == FormulaType::CARTESIAN_LINEAR) {
		drawEigenvectors(Vec2(0.0f), linearFormulaMatrixEigenvectors, 1.0f, 0.0f);
	}
	for (const auto& graph : implicitFunctionGraphs) {
		drawImplicitFunctionGraph(graph.formulaInput->input, graph.color, *graph.formulaInput);
	}

	linearizationToolUpdate(linearizationToolState, fixedPoints);

	ImPlot::PopStyleColor();

	ImPlot::EndPlot();
}

void Continous2dSystemVisualization::plotStreamlines(
	const PlotCompiler& plotCompiler, 
	const Runtime::LoopFunction& f0, 
	const Runtime::LoopFunction& f1,
	FormulaType formulaType,
	const StreamlineSettings& settings) {

	const auto plotRect = ImPlot::GetPlotLimits();

	const auto capacity = 24;
	const auto stepCount = 10;
	LoopFunctionArray input(plotCompiler.runtimeVariables.size());
	LoopFunctionArray outputX(1);
	LoopFunctionArray outputY(1);
	std::vector<float> streamlineLength;
	std::vector<i32> streamlinePointCount;
	const auto posCount = stepCount + 1;
	Array2d<Vec2> streamlineIndexToPos(capacity, posCount);

	auto convertInputFromCartesianToPloar = [](LoopFunctionArray& input) {
		for (auto dataUnitsOfBlockBegin = input.data();
			dataUnitsOfBlockBegin < input.data() + input.dataUnitsOccupiedByBlocks();
			dataUnitsOfBlockBegin += input.valuesPerBlock()) {

			const auto x = dataUnitsOfBlockBegin[VARIABLE_X_INDEX_IN_BLOCK];
			const auto y = dataUnitsOfBlockBegin[VARIABLE_Y_INDEX_IN_BLOCK];

			dataUnitsOfBlockBegin[VARIABLE_A_INDEX_IN_BLOCK] = _mm256_atan2_ps(y, x);
			dataUnitsOfBlockBegin[VARIABLE_R_INDEX_IN_BLOCK] = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(x, x), _mm256_mul_ps(y, y)));
		}
	};

	auto convertOutputFromPloarToCartesian = [](const LoopFunctionArray& cartesianInput, const LoopFunctionArray& polarInput, LoopFunctionArray& outputX, LoopFunctionArray& outputY) {

		for (int i = 0; i < cartesianInput.blockCount(); i++) {
			const auto x = cartesianInput(i, VARIABLE_X_INDEX_IN_BLOCK);
			const auto y = cartesianInput(i, VARIABLE_Y_INDEX_IN_BLOCK);
			const auto r = polarInput(i, VARIABLE_R_INDEX_IN_BLOCK);
			auto& dadt = outputX(i, 0);
			auto& drdt = outputY(i, 0);

			const auto dvdt = polarDerivativeToCartesianDerivative(dadt, drdt, Vec2(x, y), r);
			outputX(i, 0) = dvdt.x;
			outputY(i, 0) = dvdt.y;
		}
	};

	LoopFunctionArray polarInput;
	auto evaluateFormula = [&](const LoopFunctionArray& cartesianInput, LoopFunctionArray& cartesianOutputX, LoopFunctionArray& cartesianOutputY) {
		if (formulaType == FormulaType::POLAR) {
			polarInput.copyIntoItself(cartesianInput);
			convertInputFromCartesianToPloar(polarInput);
			f0(polarInput, outputX);
			f1(polarInput, outputY);
			convertOutputFromPloarToCartesian(cartesianInput, polarInput, outputX, outputY);
		} else {
			f0(input, outputX);
			f1(input, outputY);
		}
	};

	auto runIntegration = [&]() {
		streamlineLength.clear();
		streamlineLength.resize(input.blockCount(), 0.0f);
		streamlinePointCount.clear();
		streamlinePointCount.resize(input.blockCount());

		outputX.resizeWithoutCopy(input.blockCount());
		outputY.resizeWithoutCopy(input.blockCount());
		for (i32 stepIndex = 0; stepIndex < stepCount; stepIndex++) {
			evaluateFormula(input, outputX, outputY);

			const auto step = settings.spacing / 5;
			for (i32 streamLineIndex = 0; streamLineIndex < input.blockCount(); streamLineIndex++) {
				const Vec2 oldPos(
					input(streamLineIndex, VARIABLE_X_INDEX_IN_BLOCK),
					input(streamLineIndex, VARIABLE_Y_INDEX_IN_BLOCK)
				);
				float dx = outputX(streamLineIndex, 0) * step;
				float dy = outputY(streamLineIndex, 0) * step;

				const auto maxLength = settings.spacing * 0.9f;
				const auto newLength = streamlineLength[streamLineIndex] + sqrt(dx * dx + dy * dy);

				if (streamlineLength[streamLineIndex] > maxLength) {
					streamlineLength[streamLineIndex] = newLength; // Still update for later use. // TODO: Why bother calculating it if you could just take the value of the derivative as the approximation of the length.
					continue;
				}

				const auto newLengthLeft = maxLength - newLength;
				const bool tooLong = newLengthLeft <= 0;
				if (tooLong) {
					// Take longest possible step that doesn't exceed the max length.
					const double currentLengthLeft = maxLength - streamlineLength[streamLineIndex];
					const auto normalized = Vec2(dx, dy).normalized() * currentLengthLeft;
					dx = normalized.x;
					dy = normalized.y;
				}

				// Set the new length even if tooLong, so that if tooLong the next doesn't continue.
				streamlineLength[streamLineIndex] = newLength;

				input(streamLineIndex, VARIABLE_X_INDEX_IN_BLOCK) += dx;
				input(streamLineIndex, VARIABLE_Y_INDEX_IN_BLOCK) += dy;
				const Vec2 newPos(
					input(streamLineIndex, VARIABLE_X_INDEX_IN_BLOCK),
					input(streamLineIndex, VARIABLE_Y_INDEX_IN_BLOCK)
				);
				if (stepIndex == 0) {
					streamlineIndexToPos(streamLineIndex, 0) = oldPos;
				}
				streamlineIndexToPos(streamLineIndex, stepIndex + 1) = newPos;
				streamlinePointCount[streamLineIndex] = stepIndex + 2;
			}
		}

		ImPlot::PushPlotClipRect();
		for (i64 streamlineIndex = 0; streamlineIndex < input.blockCount(); streamlineIndex++) {
			const auto color = Color3::scientificColoring(streamlineLength[streamlineIndex], 0.0f, 5.0f);
			const auto colorInt = plotColorToColorInt(color);

			for (i64 pointIndex = 0; pointIndex < streamlinePointCount[streamlineIndex] - 1; pointIndex++) {
				const auto start = streamlineIndexToPos(streamlineIndex, pointIndex);
				const auto end = streamlineIndexToPos(streamlineIndex, pointIndex + 1);
				plotAddLine(start, end, colorInt);
			}

			const auto count = streamlinePointCount[streamlineIndex];
			const auto lastPos = streamlineIndexToPos(streamlineIndex, count - 1);
			const auto derivative = lastPos - streamlineIndexToPos(streamlineIndex, count - 2);
			const auto angle = derivative.angle();
			const auto a = Vec2::oriented(angle + 0.4f);
			const auto b = Vec2::oriented(angle - 0.4f);
			if (derivative.lengthSq() == 0.0f) {
				continue;
			}
			const auto arrowheadLength = settings.spacing * 0.1f;
			plotAddLine(lastPos, lastPos - a.normalized() * arrowheadLength, colorInt);
			plotAddLine(lastPos, lastPos - b.normalized() * arrowheadLength, colorInt);

		}
		ImPlot::PopPlotClipRect();

		input.clear();
	};

	std::vector<float> inputBlock = plotCompiler.loopFunctionVariablesBlock;
	const auto minX = i32(floor(plotRect.X.Min / settings.spacing));
	const auto minY = i32(floor(plotRect.Y.Min / settings.spacing));
	const auto maxX = i32(ceil(plotRect.X.Max / settings.spacing));
	const auto maxY = i32(ceil(plotRect.Y.Max / settings.spacing));
	for (i32 xi = minX; xi <= maxX; xi++) {
		for (i32 yi = minY; yi <= maxY; yi++) {
			const auto x = float(xi) * settings.spacing;
			const auto y = float(yi) * settings.spacing;

			inputBlock[VARIABLE_X_INDEX_IN_BLOCK] = x;
			inputBlock[VARIABLE_Y_INDEX_IN_BLOCK] = y;
			input.append(inputBlock);
			if (input.blockCount() == capacity) {
				runIntegration();
			}
		}
	}
	runIntegration();
}

void Continous2dSystemVisualization::plotTestPoints() {
	auto calculateDerivative = [&](Vec2 pos, float t) -> Vec2 {
		return sampleVectorField(pos);
	};

	if (!paused) {
		float dt = 1.0f / 60.0f;
		for (auto& point : testPoints) {
			point.history.push_back(point.pos);
			point.pos = rungeKutta4Step(calculateDerivative, point.pos, 0.0f, dt);
		}
	}

	{
		const auto testPointsData = reinterpret_cast<const u8*>(testPoints.data());
		ImPlot::PushStyleColor(ImPlotCol_MarkerFill, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImPlot::PlotScatter(
			"test points",
			reinterpret_cast<const float*>(testPointsData + offsetof(TestPoint, pos.x)),
			reinterpret_cast<const float*>(testPointsData + offsetof(TestPoint, pos.y)),
			testPoints.size(),
			0,
			0,
			sizeof(TestPoint)
		);
		for (const auto& p : testPoints) {
			plotVec2Line("test points trajectories", p.history);
		}
		ImPlot::PopStyleColor();
		ImPlot::PopStyleColor();
		ImPlot::PopStyleColor();
	}

	Input::ignoreImGuiWantCapture = true;
	if (selectedToolType == ToolType::SPAWN_TEST_POINTS && Input::isMouseButtonDown(MouseButton::MIDDLE)) {
		const auto mousePos = ImPlot::GetPlotMousePos();
		const auto p = TestPoint{ Vec2(mousePos.x, mousePos.y) };
		testPoints.push_back(p);
	}
	Input::ignoreImGuiWantCapture = false;

	// TODO: Something like this https://github.com/matplotlib/matplotlib/blob/main/lib/matplotlib/streamplot.py
	if (spawnPointsOnBoundaryNextFrame) {
		spawnPointsOnBoundaryNextFrame = false;

		const auto limits = ImPlot::GetPlotLimits();
		const auto spacing = 0.3f;
		const auto samplesX = i32((limits.X.Max - limits.X.Min) / spacing);
		for (i64 i = 0; i < samplesX; i++) {
			const float t = float(i) / float(samplesX - 1);
			const float x = lerp(limits.X.Min, limits.X.Max, t);
			testPoints.push_back(TestPoint{ Vec2(x, limits.Y.Min) });
			testPoints.push_back(TestPoint{ Vec2(x, limits.Y.Max) });
		}

		const auto samplesY = i32((limits.Y.Max - limits.Y.Min) / spacing);
		for (i64 i = 0; i < samplesY; i++) {
			const float t = float(i) / float(samplesY - 1);
			const float y = lerp(limits.Y.Min, limits.Y.Max, t);
			testPoints.push_back(TestPoint{ Vec2(limits.X.Min, y) });
			testPoints.push_back(TestPoint{ Vec2(limits.X.Max, y) });
		}
	}

	if (spawnGridOfPointsNextFrame) {
		spawnGridOfPointsNextFrame = false;
		const auto limits = ImPlot::GetPlotLimits();
		const auto spacing = 0.3f;
		const auto samplesX = i32((limits.X.Max - limits.X.Min) / spacing);
		const auto samplesY = i32((limits.Y.Max - limits.Y.Min) / spacing);
		i64 xiMin = 0;
		i64 xiMax = samplesX;
		i64 yiMin = 0;
		i64 yiMax = samplesY;

		for (i64 xi = 0; xi < samplesX; xi++) {
			for (i64 yi = 0; yi < samplesY; yi++) {
				const float xt = float(xi) / float(samplesX - 1);
				const float yt = float(yi) / float(samplesY - 1);
				const float x = lerp(limits.X.Min, limits.X.Max, xt);
				const float y = lerp(limits.Y.Min, limits.Y.Max, yt);
				testPoints.push_back(TestPoint{ Vec2(x, y) });
			}
		}
	}
}

void Continous2dSystemVisualization::plotFixedPoints() {
	fixedPoints.clear();

	if (!formulaInput0.loopFunction.has_value() || !formulaInput1.loopFunction.has_value()) {
		return;
	}

	const auto steps = 100;

	auto computeGraphForIntersection = [&](
		const Runtime::LoopFunction& function,
		std::vector<MarchingSquares3Line>& lines,
		Array2d<MarchingSquaresGridCell>& gridCellToLines,
		std::vector<Vec2>& graphEndpoints) {

		LoopFunctionArray output(1);
		computeLoopFunctionOnVisibleRegion(function, output, steps, steps);

		const auto grid = Span2d<const float>(output.data()->m256_f32, steps, steps);
		marchingSquares3(lines, gridCellToLines.span2d(), grid, 0.0f, true);

		const auto limits = plotLimits();
		rescaleMarchingSquaresLinesAndConvertToVectorOfEndpoints(lines, graphEndpoints, Vec2(grid.size()), limits.min, limits.max);
	};

	std::vector<MarchingSquares3Line> yGraphLines;
	Array2d<MarchingSquaresGridCell> yGraphGridCellToLines(steps - 1, steps - 1);
	std::vector<Vec2> yGraphEndpoints;
	computeGraphForIntersection(*formulaInput0.loopFunction, yGraphLines, yGraphGridCellToLines, yGraphEndpoints);
	std::vector<MarchingSquares3Line> xGraphLines;
	Array2d<MarchingSquaresGridCell> xGraphGridCellToLines(steps - 1, steps - 1);
	std::vector<Vec2> xGraphEndpoints;
	computeGraphForIntersection(*formulaInput1.loopFunction, xGraphLines, xGraphGridCellToLines, xGraphEndpoints);

	auto checkIntersection = [&](MarchingSquares3Line& xLine, i32 yLineIndex) {
		const auto& yLine = yGraphLines[yLineIndex];
		const auto intersection = LineSegment{ xLine.a, xLine.b }.intersection(LineSegment{ yLine.a, yLine.b });
		if (intersection.has_value()) {
			fixedPoints.push_back(*intersection);
		}
	};

	for (auto& xLine : xGraphLines) {
		const auto& yLinesInTheSameBoxAsTheXLine = yGraphGridCellToLines(xLine.gridIndex.x, xLine.gridIndex.y);
		if (yLinesInTheSameBoxAsTheXLine.line1Index != MarchingSquaresGridCell::EMPTY) {
			checkIntersection(xLine, yLinesInTheSameBoxAsTheXLine.line1Index);
		}
		if (yLinesInTheSameBoxAsTheXLine.line2Index != MarchingSquaresGridCell::EMPTY) {
			checkIntersection(xLine, yLinesInTheSameBoxAsTheXLine.line2Index);
		}
	}

	if (drawNullclines) {
		ImPlot::PushStyleColor(ImPlotCol_Line, Vec4(Color3::RED));
		plotVec2LineSegments("x'=0", xGraphEndpoints);
		ImPlot::PopStyleColor();

		ImPlot::PushStyleColor(ImPlotCol_Line, Vec4(Color3::GREEN));
		plotVec2LineSegments("y'=0", yGraphEndpoints);
		ImPlot::PopStyleColor();
	}

	plotVec2Scatter("fixed points", fixedPoints);

	for (const auto& fixedPoint : fixedPoints) {
		const auto jacobian = calculateJacobian(fixedPoint);
		if (formulaType != FormulaType::CARTESIAN_LINEAR) {
			drawEigenvectors(fixedPoint, computeEigenvectors(jacobian.transposed()), 0.2f, 0.001f);
		}
	}
}

Vec2 Continous2dSystemVisualization::sampleVectorField(Vec2 input) {
	// Could make this explicitly take the formulaInputs and the type as arguments.
	if (!formulaInput0.loopFunction.has_value() || !formulaInput1.loopFunction.has_value()) {
		return Vec2(0.0f);
	}
	auto& state = sampleVectorFieldState;
	
	state.input.clear();
	for (i32 i = 0; i < plotCompiler.loopFunctionVariablesBlock.size(); i++) {
		__m256 v;
		v.m256_f32[0] = plotCompiler.loopFunctionVariablesBlock[i];
		state.input.push_back(v);
	}
	
	const auto cartesianPos = input;

	float a;
	float r;
	if (formulaType == FormulaType::POLAR) {
		a = atan2(input.y, input.x);
		r = input.length();
		input = Vec2(a, r);
	}

	state.input[VARIABLE_0_INDEX_IN_BLOCK].m256_f32[0] = input.x;
	state.input[VARIABLE_1_INDEX_IN_BLOCK].m256_f32[0] = input.y;
	__m256 output;
	(*formulaInput0.loopFunction)(state.input.data(), &output, 1);
	const auto d0dt = output.m256_f32[0];
	(*formulaInput1.loopFunction)(state.input.data(), &output, 1);
	const auto d1dt = output.m256_f32[0];

	if (formulaType == FormulaType::POLAR) {
		const auto dadt = d0dt;
		const auto drdt = d1dt;
		return polarDerivativeToCartesianDerivative(dadt, drdt, cartesianPos, r);
	}
	return Vec2(d0dt, d1dt);
}

void Continous2dSystemVisualization::settings() {
	auto printComplex = [](std::ostream& os, Complex32 c) {
		os << c.real();
		if (c.imag() != 0.0f) {
			os << " + " << c.imag() << "i";
		}
	};

	auto printComplexVector = [&](std::ostream& os, const Vec2T<Complex32>& v) {
		os << "[";
		printComplex(os, v.x);
		os << ", ";
		printComplex(os, v.y);
		os << "]";
	};

	auto printEigenvector = [&](std::ostream& os, const Eigenvector& e) {
		printComplexVector(os, e.eigenvector);
		os << '\n';
		printComplex(os, e.eigenvalue);
		os << '\n';
	};

	ImGui::Combo("tool", reinterpret_cast<int*>(&selectedToolType), toolTypeNames);

	const auto oldFormulaType = formulaType;
	if (ImGui::Combo("formula type", reinterpret_cast<int*>(&formulaType), formulaTypeNames) && oldFormulaType != formulaType) {
		switch (formulaType) {
			using enum FormulaType;
		case CARTESIAN_LINEAR: changeFormulaTypeToCartesianLinear(); break;
		case CARTESIAN: changeFormulaTypeToCartesian(); break;
		case POLAR: changeFormulaTypeToPolar(); break;
		}
	}

	switch (formulaType) {
		using enum FormulaType;
	case CARTESIAN_LINEAR: {
		auto row0 = linearFormulaMatrix.row0();
		auto row1 = linearFormulaMatrix.row1();
		bool modified = false;
		modified |= ImGui::InputFloat2("##row0", row0.data());
		modified |= ImGui::InputFloat2("##row1", row1.data());
		linearFormulaMatrix = Mat2::fromRows(row0, row1);
		if (modified) {
			updateLinearFormula();
		}

		StringStream s;
		printEigenvector(s, linearFormulaMatrixEigenvectors[0]);
		printEigenvector(s, linearFormulaMatrixEigenvectors[1]);

		s << linearSystemTypeToString(linearSystemType(linearFormulaMatrix));
		s << '\n';
		ImGui::Text("%s", s.string().c_str());
		break;
	}
		
	case CARTESIAN:
		plotCompiler.formulaInputGui("x'=", formulaInput0);
		plotCompiler.formulaInputGui("y'=", formulaInput1);
		break;

	case POLAR:
		plotCompiler.formulaInputGui("a'=", formulaInput0);
		plotCompiler.formulaInputGui("r'=", formulaInput1);
		break;
	}

	// You could just click on the legent to disable the graphs.
	// Also you can put the settings inside the legend like in the DemoWindow Tools.
	ImGui::Checkbox("draw nullclines", &drawNullclines);

	plotCompiler.settingsWindowContent();

	if (ImGui::Button("add implicit function graph")) {
		implicitFunctionGraphs.push_back(ImplicitFunctionGraph{
			.formulaInput = plotCompiler.allocateFormulaInput(),
			.color = Color3::RED,
			.isHidden = false,
		});
	}
	const auto [f, l] = std::ranges::remove_if(
		implicitFunctionGraphs, 
		[&](ImplicitFunctionGraph& plot) -> bool {
			return implicitFunctionGraphSettings(plot);
		}
	);
	implicitFunctionGraphs.erase(f, l);

	ImGui::SeparatorText("test points");
	if (ImGui::Button("remove all")) {
		testPoints.clear();
	}
	if (ImGui::Button("spawn on boundary")) {
		spawnPointsOnBoundaryNextFrame = true;
	}
	if (ImGui::Button("spawn grid")) {
		spawnGridOfPointsNextFrame = true;
	}
	ImGui::Checkbox("paused", &paused);

	ImGui::SeparatorText("linearized point");
	linearizationToolSettings();
}

bool Continous2dSystemVisualization::examplesMenu() {
	if (ImGui::MenuItem("symmetrical saddle node")) {
		plotCompiler.setFormulaInput(formulaInput0, "x");
		plotCompiler.setFormulaInput(formulaInput1, "-y");
		changeFormulaTypeToCartesian();
		return true;
	}
	if (ImGui::MenuItem("symmetrical saddle node")) {
		plotCompiler.setFormulaInput(formulaInput0, "x");
		plotCompiler.setFormulaInput(formulaInput1, "-y");
		changeFormulaTypeToCartesian();
		return true;
	}
	if (ImGui::MenuItem("damped harmonic oscillator")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.setFormulaInput(formulaInput0, "y");
		plotCompiler.setFormulaInput(formulaInput1, "-x-ay");
		changeFormulaTypeToCartesian();
		return true;
	}
	if (ImGui::MenuItem("damped pendulum")) {
		plotCompiler.addParameterIfNotExists("b");
		plotCompiler.setFormulaInput(formulaInput0, "y");
		plotCompiler.setFormulaInput(formulaInput1, "-sin(x) - by");
		changeFormulaTypeToCartesian();
		/*
		The fixed point on top (x = +-pi) is always a saddle point for all positive values of b.
		b = 0 no damping, the bottom (x = 0) is a center, can be verified that it is a nonlinear center by finding the conserved quantity and using the appropriate theorem.
		b in (0, 2) underdamped, the bottom is a stable spiral
		b = 2 critically damped, the center is a degenerate stable node
		b > 0 overdamped, the bottom is a stable node.
		The system is not conservative, because there are attractors.

		The peroid of an undamped pendulum can be approximated by:
		Calculating the equation for velocity from conservation of energy. From this you get an equation for dx/dt so to get the period you have seperate the variables to get dt = f(x) dx and then integrate. This expression can be expressued using the eliptic integral of the first kind. K(m) = S((1-msin(x)^2, dx)^(-1/2). Which can the be expanded using the binomial series into a polynomial and integrated term by term.
		*/
		return true;
	}
	if (ImGui::MenuItem("pendulum driven by a constant torque")) {
		plotCompiler.addParameterIfNotExists("b");
		plotCompiler.setFormulaInput(formulaInput0, "y");
		plotCompiler.setFormulaInput(formulaInput1, "-sin(x) + b");
		/*
		The fixed points satisfy y = 0 and sin(x) = b. The latter expression can be visualzed as the points of intersection of a line with a circle. Then it's clear that the solution only exist if b in [-1, 1]. For all b in (-1, 0) U (0, 1) there are 2 solutions (when vied as a flow on a cyllidner). One is arcsin(b) the other as can be seen from the drawing is pi - arcsin(b).
		*/
		changeFormulaTypeToCartesian();
		return true;
	}

	if (ImGui::MenuItem("van der Pol oscillator")) {
		plotCompiler.setFormulaInput(formulaInput0, "y");
		plotCompiler.setFormulaInput(formulaInput1, "-x + y(1 - x^2)");
		changeFormulaTypeToCartesian();
		return true;
	}
	if (ImGui::MenuItem("dipole fixed point")) {
		// This is just the complex function x^2. You can get weird things when using complex polynomials, because whole lines get mapped to zero, because what the map does it wrap the complex plane around itself multiple times. This result in multiple lines intersecting at a single point.
		// TODO: https://mabotkin.github.io/complex/
		plotCompiler.setFormulaInput(formulaInput0, "2xy");
		plotCompiler.setFormulaInput(formulaInput1, "y^2-x^2");
		changeFormulaTypeToCartesian();
		return true;
	} 
	if (ImGui::MenuItem("gravitational well")) {
		plotCompiler.addParameterIfNotExists("d"); // distance between the bodies.
		plotCompiler.addParameterIfNotExists("m1");
		plotCompiler.addParameterIfNotExists("m2");
		plotCompiler.addParameterIfNotExists("G");
		plotCompiler.setFormulaInput(formulaInput0, "y");
		plotCompiler.setFormulaInput(formulaInput1, "Gm1/x^2 - Gm2/(d-x)^2");
		changeFormulaTypeToCartesian();
		return true;
	}
	if (ImGui::MenuItem("two-eyed monster")) {
		plotCompiler.setFormulaInput(formulaInput0, "y + y^2");
		plotCompiler.setFormulaInput(formulaInput1, "-(1/2)x + (1/5)y - xy + (6/5)y^2");
		changeFormulaTypeToCartesian();
		return true;
	}
	if (ImGui::MenuItem("parrot")) {
		plotCompiler.setFormulaInput(formulaInput0, "y + y^2");
		plotCompiler.setFormulaInput(formulaInput1, "-x + (1/5)y - xy + (6/5)y^2");
		changeFormulaTypeToCartesian();
		return true;
	}
	if (ImGui::MenuItem("Lotka-Volterra")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.addParameterIfNotExists("b");
		plotCompiler.addParameterIfNotExists("c");
		plotCompiler.addParameterIfNotExists("d");
		plotCompiler.setFormulaInput(formulaInput0, "(a-by)x");
		plotCompiler.setFormulaInput(formulaInput1, "(cx-d)y");
		changeFormulaTypeToCartesian();
		return true;
	}
	if (ImGui::MenuItem("competitive Lotka-Volterra")) {
		plotCompiler.setFormulaInput(formulaInput0, "x(3-x-2y)");
		plotCompiler.setFormulaInput(formulaInput1, "y(2-x-y)");
		changeFormulaTypeToCartesian();
		return true;
	}
	// Attractive, but not lyapunov stable r' = r(1-r) a' = r(1-cos(a)) 
	// or
	// r' = r(1-r^2) a' = 1 - cos(a)

	//if (ImGui::MenuItem("competitive Lotka-Volterra")) {
	//	// Interpolating polynomial {1, 0, 1, 0, 1, 0, 1} for the potential
	//	// 64 - (2144 x)/15 + (5348 x^2)/45 - 48 x^3 + (91 x^4)/9 - (16 x^5)/15 + (2 x^6)/45
	//	// derivative 4/45 (-1608 + 2674 x - 1620 x^2 + 455 x^3 - 60 x^4 + 3 x^5)
	//	// potential with 3 holes
	//	plotCompiler.setFormulaInput(xFormulaInput, "y");
	//	plotCompiler.setFormulaInput(yFormulaInput, "-(4/45 * (-1608 + 2674 x - 1620 x^2 + 455 x^3 - 60 x^4 + 3 x^5))");
	//	return true;
	//}

	// x' = sin(x)
	// y' = sin(xy)

	// a' = 1
	// r' = -sin(10a)^2

	// a' = sin(a)^2+1
	// r' = sin(a)

	// a' = sin(r)
	// r' = 1

	// cool orbits
	// a' = sin(5r)
	// r' = -sin(a)

	return false;
}

void Continous2dSystemVisualization::computeLoopFunctionOnVisibleRegion(const Runtime::LoopFunction& function, LoopFunctionArray& output, i32 stepsX, i32 stepsY) {

	auto& input = computeLoopFunctionOnVisibleRegionState.input;
	input.reset(plotCompiler.loopFunctionInputCount());

	const auto limits = ImPlot::GetPlotLimits();

	auto variablesBlock = plotCompiler.loopFunctionVariablesBlock;
	for (i32 yi = 0; yi < stepsX; yi++) {
		for (i32 xi = 0; xi < stepsY; xi++) {
			const auto xt = float(xi) / float(stepsX - 1);
			const auto yt = float(yi) / float(stepsY - 1);
			const auto x = lerp(limits.X.Min, limits.X.Max, xt);
			const auto y = lerp(limits.Y.Min, limits.Y.Max, yt);
			variablesBlock[VARIABLE_X_INDEX_IN_BLOCK] = x;
			variablesBlock[VARIABLE_Y_INDEX_IN_BLOCK] = y;
			input.append(variablesBlock);
		}
	}
	output.reset(1);
	output.resizeWithoutCopy(input.blockCount());

	function(input, output);
}

bool Continous2dSystemVisualization::implicitFunctionGraphSettings(ImplicitFunctionGraph& graph) {
	ImGui::PushID(&graph); // Don't think there should be any issue with the pointer being invalidated, because the invalidation wouldn't happen while using the input. Alternatively could use hash of the string name.

	bool xNotPressed = true;
	const auto open = ImGui::CollapsingHeader("", &xNotPressed);
	bool remove = !xNotPressed;
	
	if (!open) {
		ImGui::SameLine();
		ImGui::Text(graph.formulaInput->input);
		ImGui::PopID();
		return remove;
	}

	plotCompiler.formulaInputGui("0=", *graph.formulaInput);
	ImGui::ColorEdit3("color", graph.color.data());

	ImGui::PopID();
	return remove;
}

void Continous2dSystemVisualization::drawImplicitFunctionGraph(
	const char* label, 
	Vec3 color,
	const PlotCompiler::FormulaInput& formula) {

	if (!formula.loopFunction.has_value()) {
		return;
	}

	std::vector<Vec2> lines;
	calculateImplicitFunctionGraph(*formula.loopFunction, lines);
	ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color.x, color.y, color.z, 1.0f));
	plotVec2LineSegments(label, lines);
	ImPlot::PopStyleColor();
}

void Continous2dSystemVisualization::calculateImplicitFunctionGraph(const Runtime::LoopFunction& function, std::vector<Vec2>& out) {
	const auto limits = ImPlot::GetPlotLimits();

	LoopFunctionArray output(1);
	const auto steps = 100;
	computeLoopFunctionOnVisibleRegion(function, output, steps, steps);
	const auto grid = Span2d<const float>(output.data()->m256_f32, steps, steps);

	std::vector<MarchingSquaresLine> marchingSquaresOutput;
	marchingSquares2(marchingSquaresOutput, grid, 0.0f, true);

	for (auto& segment : marchingSquaresOutput) {
		auto scale = [&](Vec2 pos) -> Vec2 {
			pos /= Vec2(grid.size());
			pos.x = lerp(limits.X.Min, limits.X.Max, pos.x);
			pos.y = lerp(limits.Y.Min, limits.Y.Max, pos.y);
			return pos;
		};
		const Vec2 a = scale(segment.a);
		const Vec2 b = scale(segment.b);
		out.push_back(a);
		out.push_back(b);
	}
}

void Continous2dSystemVisualization::drawEigenvectors(Vec2 origin, const std::array<Eigenvector, 2>& eigenvectors, float scale, float complexPartTolerance) {
	ImPlot::PushPlotClipRect();
	auto drawEigenvector = [&origin, &scale](const Vec2T<Complex32>& v) {
		// Could multiple by the eigenvalue.
		plotAddArrowOriginDirection(
			origin,
			Vec2(v.x.real(), v.y.real()).normalized() * scale,
			Color3::RED,
			0.1f
		);
	};

	// In 2d either both vectors are real or both complex.
	if (std::abs(eigenvectors[0].eigenvalue.imag()) <= complexPartTolerance) {
		drawEigenvector(eigenvectors[0].eigenvector);
		drawEigenvector(eigenvectors[1].eigenvector);
	}
	ImPlot::PopPlotClipRect();
}

Mat2 Continous2dSystemVisualization::calculateJacobian(Vec2 p) {
	const float d = 0.001f;
	const auto v = sampleVectorField(p);
	const auto dvdx = (sampleVectorField(p + Vec2(d, 0.0f)) - v) / d;
	const auto dvdy = (sampleVectorField(p + Vec2(0.0f, d)) - v) / d;
	return Mat2(dvdx, dvdy);
}

void Continous2dSystemVisualization::linearizationToolSettings() {
	auto& s = linearizationToolState;

	ImGui::Checkbox("show", &s.show);

	if (!s.show) {
		return;
	}
	s.jacobian = calculateJacobian(s.pointToLinearlizeAbout);

	auto tableVec2Row = [](Vec2 v) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%g", v.x);
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%g", v.y);
	};

	ImGui::Text("position");
	if (ImGui::BeginTable("position", 2, ImGuiTableFlags_BordersOuterV)) {
		tableVec2Row(s.pointToLinearlizeAbout);
		ImGui::EndTable();
	}
	ImGui::Text("jacobian");
	if (ImGui::BeginTable("jacobian", 2, ImGuiTableFlags_BordersOuterV)) {
		tableVec2Row(Vec2(s.jacobian.row0()));
		tableVec2Row(Vec2(s.jacobian.row1()));
		ImGui::EndTable();
	}
	ImGui::Text("%s", linearSystemTypeToString(linearSystemType(s.jacobian)));
}

void Continous2dSystemVisualization::linearizationToolUpdate(LinearizationToolState& s, const std::vector<Vec2> fixedPoints) {
	if (!s.show) {
		return;
	}
	double x = s.pointToLinearlizeAbout.x;
	double y = s.pointToLinearlizeAbout.y;
	bool dragged = ImPlot::DragPoint(0, &x, &y, Vec4(Color3::RED));
	s.pointToLinearlizeAbout = Vec2(x, y);

	std::optional<Vec2> closestFixedPoint;
	float closestFixedPointDistance = std::numeric_limits<float>::infinity();
	for (const auto& fixedPoint : fixedPoints) {
		const auto fixedPointScreenCoordinates = Vec2(ImPlot::PlotToPixels(ImVec2(fixedPoint)));
		const auto selectedPointScreenCoordinates = Vec2(ImPlot::PlotToPixels(ImVec2(s.pointToLinearlizeAbout)));
		
		const auto d = selectedPointScreenCoordinates.distanceSquaredTo(fixedPointScreenCoordinates);
		if (d < closestFixedPointDistance) {
			closestFixedPoint = fixedPoint;
			closestFixedPointDistance = d;
		}
	}

	const auto snapToFixedPoint = closestFixedPoint.has_value() && closestFixedPointDistance < 150.0f;
	if (snapToFixedPoint) {
		s.pointToLinearlizeAbout = *closestFixedPoint;
	}
}

void Continous2dSystemVisualization::changeFormulaTypeToCartesian() {
	formulaType = FormulaType::CARTESIAN;
	plotCompiler.variables[VARIABLE_0_INDEX_IN_BLOCK].name = "x";
	plotCompiler.variables[VARIABLE_1_INDEX_IN_BLOCK].name = "y";
}

void Continous2dSystemVisualization::changeFormulaTypeToCartesianLinear() {
	changeFormulaTypeToCartesian();
	formulaType = FormulaType::CARTESIAN_LINEAR;
	updateLinearFormula();
}

void Continous2dSystemVisualization::changeFormulaTypeToPolar() {
	formulaType = FormulaType::POLAR;
	plotCompiler.variables[VARIABLE_0_INDEX_IN_BLOCK].name = "a";
	plotCompiler.variables[VARIABLE_1_INDEX_IN_BLOCK].name = "r";
}

void Continous2dSystemVisualization::updateLinearFormula() {
	// Using a stream so this doesn't break if there is scientific notation
	StringStream formula;
	formula << std::setprecision(1000);

	formula << linearFormulaMatrix(0, 0) << "x + " << linearFormulaMatrix(1, 0) << "y";
	plotCompiler.setFormulaInput(formulaInput0, formula.string());

	formula.string().clear();
	formula << linearFormulaMatrix(0, 1) << "x + " << linearFormulaMatrix(1, 1) << "y";
	plotCompiler.setFormulaInput(formulaInput1, formula.string());

	linearFormulaMatrixEigenvectors = computeEigenvectors(linearFormulaMatrix);
}

#include <glad/glad.h>
#include <engine/Math/Random.hpp>

void Continous2dSystemVisualization::BasinOfAttractionWindow::update(
	const Continous2dSystemVisualization& state,
	Renderer2d& renderer2d) {
	if (!shaderProgram.has_value()) {
		return;
	}

	if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
		const auto cursorPos = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
		if (grabStartPosWorldSpace.has_value()) {
			const auto differece = *grabStartPosWorldSpace - cursorPos;
			camera.pos += differece;
		} else {
			grabStartPosWorldSpace = cursorPos;
		}
	}
	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
		grabStartPosWorldSpace = std::nullopt;
	}

	if (const auto scroll = Input::scrollDelta(); scroll != 0.0f) {
		const auto cursorPosBeforeScroll = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
		const auto scrollSpeed = 15.0f * abs(scroll);
		const auto scrollIncrement = pow(scrollSpeed, 1.0f / 60.0f);
		if (scroll > 0.0f) camera.zoom *= scrollIncrement;
		else camera.zoom /= scrollIncrement;
	
		camera.pos -= (Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace() - cursorPosBeforeScroll);
	}

	renderer2d.fullscreenQuad2dPtVerticesVao.bind();
	const auto bounds = camera.aabb();
	shaderProgram->set("viewMin", bounds.min);
	shaderProgram->set("viewMax", bounds.max);
	static int iterations = 0;
	ImGui::Begin("settings");
	ImGui::SliderInt("iterations", &iterations, 0, 10000);
	ImGui::End(); 
	shaderProgram->set("iterations", iterations);
	
	//if (state.fixedPoints.size())
	const auto count = std::min(state.fixedPoints.size(), size_t(4));
	for (int i = 0; i < count; i++) {
		const auto index = "[" + std::to_string(i) + "]";
		shaderProgram->set("fixedPoints" + index, state.fixedPoints[i]);
		//const auto color = Vec3(state.fixedPoints[i].x, state.fixedPoints[i].y, 0.0f);
		const auto color = Color3::fromHsv(float(i) / 4, 1.0f, 1.0f);
		shaderProgram->set("fixedPointsColors" + index, color);
		renderer2d.shapeRenderer.circleInstances.push_back(CircleInstance{
			.transform = camera.makeTransform(state.fixedPoints[i], 0.0f, Vec2(1.0f)),
			.color = color,
			.smoothing = 0.05f,
			.width = 0.1f,
		});
	}
	shaderProgram->set("fixedPointsCount", i32(count));

	for (int i = 0; i < state.plotCompiler.parameters.size(); i++) {
		const auto index = state.plotCompiler.parameterIndexToLoopFunctionVariableIndex(i);
		shaderProgram->set("v" + std::to_string(index), state.plotCompiler.loopFunctionVariablesBlock[index]);
	}
	shaderProgram->use();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glUseProgram(0);

	//renderer2d.update();
}

void Continous2dSystemVisualization::BasinOfAttractionWindow::recompileShader(
	Continous2dSystemVisualization& state,
	Renderer2d& renderer2d) {

	// This doesn't work well, because many interation are required to converge to the fixed point. It looks like a fractal, but after many iteration it coverges into simple shapes. Look at the shadertoy below.
	StringStream s;
	s << "#version 430 core\n";
	s << "in vec2 fragmentTexturePosition;\n";
	s << "out vec4 fragColor;\n";
	for (int i = 0; i < state.plotCompiler.parameters.size(); i++) {
		const auto index = state.plotCompiler.parameterIndexToLoopFunctionVariableIndex(i);
		s << "uniform float v" << index << ";\n";
	}
	s << "uniform vec2 viewMin;\n";
	s << "uniform vec2 viewMax;\n";
	s << "uniform int iterations;\n";
	s << "uniform vec2 fixedPoints[4];\n";
	s << "uniform vec3 fixedPointsColors[4];\n";
	s << "uniform int fixedPointsCount;\n";

	//s << R"(
	//vec3 hsv2rgb(vec3 c) {
	//	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	//	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	//	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
	//}
	//)";

	//s << "void main() {\n";
	//{
	//	s << "vec2 worldPos = mix(viewMin, viewMax, fragmentTexturePosition);\n";
	//	s << "float v" << X_VARIABLE_INDEX_IN_BLOCK << " = worldPos.x;\n";
	//	s << "float v" << Y_VARIABLE_INDEX_IN_BLOCK << " = worldPos.y;\n";
	//	s << "for (int i = 0; i < iterations; i++) {\n";
	//	s << "float dxdt;\n";
	//	{
	//		s << "{\n";
	//		if (!state.plotCompiler.tryCompileGlsl(s, state.xFormulaInput)) {
	//			shaderProgram = std::nullopt;
	//			return;
	//		}
	//		s << "dxdt = result;\n";
	//		s << "}\n";
	//	}
	//	s << "float dydt;\n";
	//	{
	//		s << "{\n";
	//		if (!state.plotCompiler.tryCompileGlsl(s, state.yFormulaInput)) {
	//			shaderProgram = std::nullopt;
	//			return;
	//		}
	//		s << "dydt = result;\n";
	//		s << "}\n";
	//	}
	//	s << "v" << X_VARIABLE_INDEX_IN_BLOCK << " += dxdt * 0.01;\n";
	//	s << "v" << Y_VARIABLE_INDEX_IN_BLOCK << " += dydt * 0.01;\n";

	//	s << "vec2 outPos = vec2(v" << X_VARIABLE_INDEX_IN_BLOCK << ", v" << Y_VARIABLE_INDEX_IN_BLOCK << ");\n";

	//	s << R"(
	//	bool found = false;
	//	vec3 color = vec3(0.0);
	//	for (int i = 0; i < fixedPointsCount; i++) {
	//		if (distance(fixedPoints[i], outPos) < 0.05) {
	//			color = fixedPointsColors[i];
	//			found = true;
	//			break;
	//		}
	//	}
	//	if (found) {
	//		fragColor = vec4(hsv2rgb(vec3(float(i) * 0.2, 1.0, 1.0)), 1.0);
	//		break;
	//	}

	//	)";

	//	s << "}\n";

	//	//s << "outPos = mod(outPos, 1.0);\n";
	//	//s << "fragColor = vec4(outPos.x, outPos.y, 0, 1);" << "\n";
	//	////s << "fragColor = vec4(fragmentTexturePosition.x, fragmentTexturePosition.y, 0, 1);\n";
	//	//s << "}\n";
	//}

	s << "void main() {\n";
	{
		s << "vec2 worldPos = mix(viewMin, viewMax, fragmentTexturePosition);\n";
		s << "float v" << VARIABLE_X_INDEX_IN_BLOCK << " = worldPos.x;\n";
		s << "float v" << VARIABLE_Y_INDEX_IN_BLOCK << " = worldPos.y;\n";
		s << "for (int i = 0; i < iterations; i++) {\n";
		s << "float dxdt;\n";
		{
			s << "{\n";
			if (!state.plotCompiler.tryCompileGlsl(s, state.formulaInput0)) {
				shaderProgram = std::nullopt;
				return;
			}
			s << "dxdt = result;\n";
			s << "}\n";
		}
		s << "float dydt;\n";
		{
			s << "{\n";
			if (!state.plotCompiler.tryCompileGlsl(s, state.formulaInput1)) {
				shaderProgram = std::nullopt;
				return;
			}
			s << "dydt = result;\n";
			s << "}\n";
		}
		s << "v" << VARIABLE_X_INDEX_IN_BLOCK << " += dxdt * 0.01;\n";
		s << "v" << VARIABLE_Y_INDEX_IN_BLOCK << " += dydt * 0.01;\n";
		s << "}\n";
		s << "vec2 outPos = vec2(v" << VARIABLE_X_INDEX_IN_BLOCK << ", v" << VARIABLE_Y_INDEX_IN_BLOCK << ");\n";
		//s << "outPos = mod(outPos, 1.0);\n";
		//s << "fragColor = vec4(outPos.x, outPos.y, 0, 1);" << "\n";
		////s << "fragColor = vec4(fragmentTexturePosition.x, fragmentTexturePosition.y, 0, 1);\n";
		//s << "}\n";
	}
	s << R"(
	vec3 color = vec3(0.0);
	for (int i = 0; i < fixedPointsCount; i++) {
		if (distance(fixedPoints[i], outPos) < 0.05) {
			color = fixedPointsColors[i];
			break;
		}
	}
	fragColor = vec4(color, 1.0);
	)";

	//s << "vec2 outPos = vec2(v" << X_VARIABLE_INDEX_IN_BLOCK << ", v" << Y_VARIABLE_INDEX_IN_BLOCK << ");\n";
	//s << "outPos = mod(outPos, 1.0);\n";
	//s << "fragColor = vec4(outPos.x, outPos.y, 0, 1);" << "\n";
	//s << "fragColor = vec4(fragmentTexturePosition.x, fragmentTexturePosition.y, 0, 1);\n";

	s << "}\n";
	std::cout << s.string() << '\n';
	auto result = ShaderProgram::fromSource(renderer2d.fullscreenQuadVertSource, s.string());
	if (!result.has_value()) {
		shaderProgram = std::nullopt;
		std::cout << result.error();
		ASSERT_NOT_REACHED();
		return;
	}
	shaderProgram = std::move(result.value());
	/*renderer2d.shapeRenderer.circleInstances.push_back(CircleInstance{
		.color = Vec2()
	})*/
	/*
	Shadertoy basin of attraction of lotka volterra model of competition.

	void mainImage( out vec4 fragColor, in vec2 fragCoord )
	{
		// Normalized pixel coordinates (from 0 to 1)
		vec2 uv = fragCoord/iResolution.xy;
    
		vec2 viewMin = vec2(-10);
		vec2 viewMax = vec2(10);
		vec2 worldPos = mix(viewMin, viewMax, uv);
		float x = worldPos.x;
		float y = worldPos.y;
		for (int i = 0; i < 10000; i++) {
			//float dxdt = 2.0 * x * y;
			//float dydt = y * y - x * x;
			float dxdt = x * (3.0 - x - 2.0 * iMouse.x / iResolution.x * y);
			float dydt = y * (2.0 - x - y);
			x += dxdt * 0.001;
			y += dydt * 0.001;
		}
		vec2 outPos = vec2(x, y);
		outPos = mod(outPos, 1.0);
		fragColor = vec4(outPos.x, outPos.y, 0, 1);
	}
	*/
}
