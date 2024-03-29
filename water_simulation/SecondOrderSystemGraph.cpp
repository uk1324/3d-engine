#include <water_simulation/SecondOrderSystemGraph.hpp>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/Color.hpp>
#include <water_simulation/PlotUtils.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/MarchingSquares.hpp>
#include <engine/Math/Utils.hpp>
#include <engine/Math/LineSegment.hpp>
#include <Gui.hpp>
#include <Array2d.hpp>
#include <iomanip>

// TODO: Could graph the potentials of conservative/irrotational fields.

// TODO: Could allow intersecting a sufrace with a plane. The result would just be the sum of the intersection with each triangle.

// TODO: To find the fixed points could compute the abs() of all the values then find the local minima. If the minima are near zero then I could run root finding

static constexpr auto X_VARIABLE_INDEX_IN_BLOCK = 0;
static constexpr auto Y_VARIABLE_INDEX_IN_BLOCK = 1;

SecondOrderSystemGraph::SecondOrderSystemGraph()
	: xFormulaInput(*plotCompiler.allocateFormulaInput())
	, yFormulaInput(*plotCompiler.allocateFormulaInput()) {

	plotCompiler.variables.push_back(Variable{ .name = "x" });
	plotCompiler.variables.push_back(Variable{ .name = "y" });
}

void plotLine(const char* label, const std::vector<Vec2>& vs) {
	const auto pointsData = reinterpret_cast<const float*>(vs.data());
	ImPlot::PlotLine(label, pointsData, pointsData + 1, vs.size(), 0, 0, sizeof(float) * 2);
}

void SecondOrderSystemGraph::update() {
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
}

#include <engine/Math/IntersectLineSegments.hpp>

void SecondOrderSystemGraph::derivativePlot() {
	// ImPlotFlags_CanvasOnly
	if (!ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		return;
	}

	ImPlot::SetupAxis(ImAxis_X1, "x");
	ImPlot::SetupAxis(ImAxis_Y1, "y");

	ImPlot::PushStyleColor(ImPlotCol_PlotBg, { 0.5f, 0.5f, 0.5f, 1.0f });

	plotStreamlines();

	plotFixedPoints();

	plotTestPoints();

	if (formulaType == FormulaType::LINEAR) {
		ImPlot::PushPlotClipRect();
		// In 2d either both vectors are real or both complex.
		if (linearFormulaMatrixEigenvectors[0].eigenvalue.imag() == 0.0f) {
			auto drawEigenvector = [](const Vec2T<Complex32>& v) {
				plotAddArrow(
					Vec2(0.0f),
					Vec2(v.x.real(), v.y.real()).normalized(),
					Color3::RED,
					0.1f
				);
				};
			drawEigenvector(linearFormulaMatrixEigenvectors[0].eigenvector);
			drawEigenvector(linearFormulaMatrixEigenvectors[1].eigenvector);
		}
		ImPlot::PopPlotClipRect();
	}
	for (const auto& graph : implicitFunctionGraphs) {
		drawImplicitFunctionGraph(graph.formulaInput->input, graph.color, *graph.formulaInput);
	}

	if (xFormulaInput.loopFunction.has_value() && yFormulaInput.loopFunction.has_value()) {
		//std::vector<Vec2> xGraph;
		//calculateImplicitFunctionGraph(*xFormulaInput.loopFunction, xGraph);
		//std::vector<Vec2> yGraph;
		//calculateImplicitFunctionGraph(*yFormulaInput.loopFunction, yGraph);

		//if (drawNullclines) {
		//	ImPlot::PushStyleColor(ImPlotCol_Line, Vec4(Color3::RED));
		//	/*plotVec2LineSegments("x'=0", xGraph);*/
		//	plotVec2Scatter("x'=0", xGraph);
		//	ImPlot::PopStyleColor();

		//	ImPlot::PushStyleColor(ImPlotCol_Line, Vec4(Color3::GREEN));
		//	plotVec2Scatter("y'=0", yGraph);
		//	ImPlot::PopStyleColor();
		//}


		//std::vector<Vec2> fixedPoints;
		////intersectLineSegments(xGraph, yGraph, fixedPoints);

		//ASSERT(xGraph.size() % 2 == 0);
		//ASSERT(yGraph.size() % 2 == 0);
		//for (i64 i = 0; i < xGraph.size(); i += 2) {
		//	const auto& x0 = xGraph[i];
		//	const auto& x1 = xGraph[i + 1];
		//	for (i64 j = 0; j < yGraph.size(); j += 2) {
		//		const auto& y0 = yGraph[j];
		//		const auto& y1 = yGraph[j + 1];

		//		const auto intersection = LineSegment{ x0, x1 }.intersection(LineSegment{ y0, y1 });
		//		if (intersection.has_value()) {
		//			fixedPoints.push_back(*intersection);
		//		}
		//	}
		//}

		//plotVec2Scatter("fixed points", fixedPoints);
		const auto steps = 200;

		auto computeGraphForIntersection = [&](
			const Runtime::LoopFunction& function,
			std::vector<MarchingSquares3Line>& marchingSquaresOutput,
			Array2d<MarchingSquaresGridCell>& gridCellToLines,
			std::vector<Vec2>& graphEndpoints) {

			LoopFunctionArray output(1);
			computeLoopFunctionOnVisibleRegion(function, output, steps, steps);
			const auto grid = Span2d<const float>(output.data()->m256_f32, steps, steps);

			marchingSquares3(marchingSquaresOutput, gridCellToLines.span2d(), grid, 0.0f, true);

			const auto limits = ImPlot::GetPlotLimits();
			for (auto& segment : marchingSquaresOutput) {
				auto scale = [&](Vec2 pos) -> Vec2 {
					pos /= Vec2(grid.size());
					pos.x = lerp(limits.X.Min, limits.X.Max, pos.x);
					pos.y = lerp(limits.Y.Min, limits.Y.Max, pos.y);
					return pos;
				};
				segment.a = scale(segment.a);
				segment.b = scale(segment.b);
				graphEndpoints.push_back(segment.a);
				graphEndpoints.push_back(segment.b);
			}
		};

		std::vector<MarchingSquares3Line> yGraphLines;
		Array2d<MarchingSquaresGridCell> yGraphGridCellToLines(steps - 1, steps - 1);
		std::vector<Vec2> yGraphEndpoints;
		computeGraphForIntersection(*yFormulaInput.loopFunction, yGraphLines, yGraphGridCellToLines, yGraphEndpoints);
		std::vector<MarchingSquares3Line> xGraphLines;
		Array2d<MarchingSquaresGridCell> xGraphGridCellToLines(steps - 1, steps - 1);
		std::vector<Vec2> xGraphEndpoints;
		computeGraphForIntersection(*xFormulaInput.loopFunction, xGraphLines, xGraphGridCellToLines, xGraphEndpoints);
		
		std::vector<Vec2> intersections;
		auto checkIntersection = [&](MarchingSquares3Line& xLine, i32 yLineIndex) {
			const auto& yLine = yGraphLines[yLineIndex];
			const auto intersection = LineSegment{ xLine.a, xLine.b }.intersection(LineSegment{ yLine.a, yLine.b });
			if (intersection.has_value()) {
				intersections.push_back(*intersection);
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

		ImPlot::PushStyleColor(ImPlotCol_Line, Vec4(Color3::RED));
		plotVec2LineSegments("x'=0", xGraphEndpoints);
		ImPlot::PopStyleColor();

		ImPlot::PushStyleColor(ImPlotCol_Line, Vec4(Color3::GREEN));
		plotVec2LineSegments("y'=0", yGraphEndpoints);
		ImPlot::PopStyleColor();

		plotVec2Scatter("fixed points", intersections);
	}

	ImPlot::PopStyleColor();

	ImPlot::EndPlot();
}

void SecondOrderSystemGraph::plotStreamlines() {
	if (!xFormulaInput.loopFunction.has_value() || !yFormulaInput.loopFunction.has_value()) {
		return;
	}
	const auto plotRect = ImPlot::GetPlotLimits();

	std::vector<float> inputBlock = plotCompiler.loopFunctionVariablesBlock;
	const auto capacity = 24;
	const auto stepCount = 15;
	LoopFunctionArray input(plotCompiler.runtimeVariables.size());
	LoopFunctionArray outputX(1);
	LoopFunctionArray outputY(1);
	std::vector<float> streamlineLength;
	std::vector<i32> streamlinePointCount;
	const auto posCount = stepCount + 1;
	Array2d<Vec2> streamlineIndexToPos(capacity, posCount);

	auto runIntegration = [&]() {
		streamlineLength.clear();
		streamlineLength.resize(input.blockCount(), 0.0f);
		streamlinePointCount.clear();
		streamlinePointCount.resize(input.blockCount());

		outputX.resizeWithoutCopy(input.blockCount());
		outputY.resizeWithoutCopy(input.blockCount());
		for (i32 stepIndex = 0; stepIndex < stepCount; stepIndex++) {
			(*xFormulaInput.loopFunction)(input, outputX);
			(*yFormulaInput.loopFunction)(input, outputY);

			const auto step = spacing / 5;
			for (i32 streamLineIndex = 0; streamLineIndex < input.blockCount(); streamLineIndex++) {
				const Vec2 oldPos(
					input(streamLineIndex, X_VARIABLE_INDEX_IN_BLOCK),
					input(streamLineIndex, Y_VARIABLE_INDEX_IN_BLOCK)
				);
				float dx = outputX(streamLineIndex, 0) * step;
				float dy = outputY(streamLineIndex, 0) * step;

				const auto maxLength = spacing * 0.9f;
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

				input(streamLineIndex, X_VARIABLE_INDEX_IN_BLOCK) += dx;
				input(streamLineIndex, Y_VARIABLE_INDEX_IN_BLOCK) += dy;
				const Vec2 newPos(
					input(streamLineIndex, X_VARIABLE_INDEX_IN_BLOCK),
					input(streamLineIndex, Y_VARIABLE_INDEX_IN_BLOCK)
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
			const auto arrowheadLength = spacing * 0.1f;
			plotAddLine(lastPos, lastPos - a.normalized() * arrowheadLength, colorInt);
			plotAddLine(lastPos, lastPos - b.normalized() * arrowheadLength, colorInt);

		}
		ImPlot::PopPlotClipRect();

		input.clear();
	};

	const auto minX = i32(floor(plotRect.X.Min / spacing));
	const auto minY = i32(floor(plotRect.Y.Min / spacing));
	const auto maxX = i32(ceil(plotRect.X.Max / spacing));
	const auto maxY = i32(ceil(plotRect.Y.Max / spacing));
	for (i32 xi = minX; xi <= maxX; xi++) {
		for (i32 yi = minY; yi <= maxY; yi++) {
			const auto x = float(xi) * spacing;
			const auto y = float(yi) * spacing;

			inputBlock[X_VARIABLE_INDEX_IN_BLOCK] = x;
			inputBlock[Y_VARIABLE_INDEX_IN_BLOCK] = y;
			input.append(inputBlock);
			if (input.blockCount() == capacity) {
				runIntegration();
			}
		}
	}
	runIntegration();
}

void SecondOrderSystemGraph::plotTestPoints() {
	if (!xFormulaInput.loopFunction.has_value() || !yFormulaInput.loopFunction.has_value()) {
		return;
	}

	std::vector<float> inputBlock = plotCompiler.loopFunctionVariablesBlock;
	std::vector<__m256> calculateDerivativeInput;
	for (i32 i = 0; i < inputBlock.size(); i++) {
		__m256 v;
		v.m256_f32[0] = inputBlock[i];
		calculateDerivativeInput.push_back(v);
	}
	auto calculateDerivative = [&](Vec2 pos, float t) -> Vec2 {
		calculateDerivativeInput[X_VARIABLE_INDEX_IN_BLOCK].m256_f32[0] = pos.x;
		calculateDerivativeInput[Y_VARIABLE_INDEX_IN_BLOCK].m256_f32[0] = pos.y;
		__m256 output;
		(*xFormulaInput.loopFunction)(calculateDerivativeInput.data(), &output, 1);
		const auto x = output.m256_f32[0];
		(*yFormulaInput.loopFunction)(calculateDerivativeInput.data(), &output, 1);
		const auto y = output.m256_f32[0];
		return Vec2(x, y);
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
			plotLine("test", p.history);
		}
		ImPlot::PopStyleColor();
		ImPlot::PopStyleColor();
		ImPlot::PopStyleColor();
	}

	Input::ignoreImGuiWantCapture = true;
	if (Input::isMouseButtonDown(MouseButton::MIDDLE)) {
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
#include <Dbg.hpp>
void SecondOrderSystemGraph::plotFixedPoints() {
	if (!xFormulaInput.loopFunction.has_value() || !yFormulaInput.loopFunction.has_value()) {
		return;
	}
	/*
	Finding zeros of a system of continous functions is hard, because for example a triangle mapped into any other closed curve (not sure if it has to be non self intersecting). Also it doesn't have to be bijective so it isn't a homeomorphism. For example you can map the triangle to a line.
	When you map a triangle then the map of the triangle doesn't have to be a subset or superset of the triangle created by the set of mapped points.

	I think an example of a map that create self intersection is the complex x^2 function. You can probably come up with weird cases by thing about the as vector fields and when the vectors change signs.

	The condition that there are vertices with sign ++, --, +-, -+ in a polygon doesn't ensure that there is a zero inside it. Also a if there is a zero inside a region it doesn't mean that the sign's have to match this pattern.
	Example
	Draw 2 straight lines intersecting at a point. Then draw a box such that the lines intersect 2 opposite lines of a box, but don't intersect the other 2. Then there is a zero inside, but the signs don't match.
	A counterexample for there other thing: https://youtu.be/rMg61nfkZ3M?feature=shared&t=420.

	If a quadrilateral that contains a solution and has the sign at vertices with the pattern specified above. The the algorithm in the video can be used to find the root to arbitrary precision.

	Not sure if this is the same algorithm as described in the video.
	https://en.wikipedia.org/wiki/Bisection_method#Generalization_to_higher_dimensions

	I wonder if the function maps convex sets (for example triangles) to other convex sets then can be algorithm be simplified, because a triangle is the smallest set convex set containing 3 points (this was said in the book by Pavel Alexandrov i think). So the map of the triangle will be a superset of the triangle made from the map of the vertices. This is wrong I think. Wouldn't it need to map the to concave sets for this to work.
	I guess if the set is convex then something like GJK can be used.

	Using interval arithmetic you can get a bounding box for the values that a rectangular region gets mapped to, but if the output region contains zero it doesn't mean the the a point from the input region gets mapped to zero, because it is just a bounding box. You would need to somehow compute the inverse to get the region. You could try using a iterative method that would try to converge the region into a region containing the root.

	One options could be to consider all the possible sign cases and based on that conservatively choose if it is possible for the root to be there. Then check all the possible spots and if they converge add then return them as roots.

	The probablem with just intersecting the graphs is that it breaks at singular points like double points or infinite points. Convervatively at saddle points a cross could be added. That is 4 lines 2 corresponding 2 each possible direction around the point could be added.

	"Generalization of the Bolzano theorem for simplices"
	Bolzano–Poincaré–Miranda theorem is closely related to important theorems in analysis and topology as well as it is an invaluable tool for verified solutions of numerical problems by means of interval arithmetic [list of references]
	*/

	// https://en.wikipedia.org/wiki/Bentley%E2%80%93Ottmann_algorithm
	//const auto jump = 5;
	//for (i32 xi = 0; xi < steps - jump; xi += jump) {
	//	for (i32 yi = 0; yi < steps - jump; yi += jump) {
	//		Vec2 p00 = calulcatePos(xi, yi);
	//		Vec2 v00(gridX(xi, yi), gridY(xi, yi));

	//		if (p00.length() < 0.001f) {
	//			int x = 5;
	//		}

	//		Vec2 p10 = calulcatePos(xi + jump, yi);
	//		Vec2 v10(gridX(xi + jump, yi), gridY(xi + jump, yi));

	//		Vec2 p01 = calulcatePos(xi, yi + jump);
	//		Vec2 v01(gridX(xi, yi + jump), gridY(xi, yi + jump));

	//		Vec2 p11 = calulcatePos(xi + jump, yi + jump);
	//		Vec2 v11(gridX(xi + jump, yi + jump), gridY(xi + jump, yi + jump));

	//		u8 signCombinations = 0b0000;
	//		auto checkSignCombinations = [&signCombinations](Vec2 v) {
	//			if (v.x >= 0.0f && v.y >= 0.0f) signCombinations |= 0b1000;
	//			if (v.x <= 0.0f && v.y >= 0.0f) signCombinations |= 0b0100;
	//			if (v.x >= 0.0f && v.y <= 0.0f) signCombinations |= 0b0010;
	//			if (v.x <= 0.0f && v.y <= 0.0f) signCombinations |= 0b0001;
	//		};
	//		checkSignCombinations(v00);
	//		checkSignCombinations(v01);
	//		checkSignCombinations(v10);
	//		checkSignCombinations(v11);

	//		if (signCombinations != 0b1111) {
	//			continue;
	//		}
	//		fixedPoints.push_back(p00);
	//		fixedPoints.push_back(p10);
	//		fixedPoints.push_back(p01);
	//		fixedPoints.push_back(p11);
	//	}
	//}

	/*const auto data = reinterpret_cast<float*>(fixedPoints.data());
	ImPlot::PlotScatter("fixed points", data, data + 1, fixedPoints.size(), 0, 0, sizeof(Vec2));*/
}

void SecondOrderSystemGraph::settings() {
	auto updateLinearFormula = [this]() {
		// Using a stream so this doesn't break if there is scientific notation
		StringStream formula;
		formula << std::setprecision(1000);

		formula << linearFormulaMatrix[0][0] << "x + " << linearFormulaMatrix[0][1] << "y";
		plotCompiler.setFormulaInput(xFormulaInput, formula.string());

		formula.string().clear();
		formula << linearFormulaMatrix[1][0] << "x + " << linearFormulaMatrix[1][1] << "y";
		plotCompiler.setFormulaInput(yFormulaInput, formula.string());

		linearFormulaMatrixEigenvectors = computeEigenvectors(linearFormulaMatrix);
	};

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

	if (ImGui::Combo("formula type", reinterpret_cast<int*>(&formulaType), formulaTypeNames)) {
		updateLinearFormula();
	}
	switch (formulaType) {
		using enum FormulaType;
	case LINEAR: {
		auto row0 = Vec2(linearFormulaMatrix[0][0], linearFormulaMatrix[1][0]);
		auto row1 = Vec2(linearFormulaMatrix[0][1], linearFormulaMatrix[1][1]);
		bool modified = false;
		modified |= ImGui::InputFloat2("##row0", row0.data());
		modified |= ImGui::InputFloat2("##row1", row1.data());
		linearFormulaMatrix = Mat2(Vec2(row0.x, row1.x), Vec2(row0.y, row1.y));
		if (modified) {
			updateLinearFormula();
		}

		StringStream s;
		printEigenvector(s, linearFormulaMatrixEigenvectors[0]);
		printEigenvector(s, linearFormulaMatrixEigenvectors[1]);

		const auto determinant = linearFormulaMatrix.det();
		const auto trace = linearFormulaMatrix[0][0] + linearFormulaMatrix[1][1];
		const auto discriminant = trace * trace - 4.0f * determinant;
		if (determinant == 0.0f) {
			s << "non isolated fixed points";
		} else if (determinant < 0.0f) {
			s << "saddle point";
		} else {
			if (trace > 0.0f) {
				if (discriminant > 0.0f) {
					s << "ustable node";
				} else if (discriminant < 0.0f) {
					s << "unstable spiral";
				} else {
					s << "unstable degenerate node";
				}
			} else if (trace < 0.0f) {
				if (discriminant > 0.0f) {
					s << "stable node";
				} else if (discriminant < 0.0f) {
					s << "stable spiral";
				} else {
					s << "stable degenerate node";
				}
			} else {
				s << "center";
			}
		}
		s << '\n';
		ImGui::Text("%s", s.string().c_str());
		break;
	}
		
	case GENERAL:
		plotCompiler.formulaInputGui("x'=", xFormulaInput);
		plotCompiler.formulaInputGui("y'=", yFormulaInput);
		break;
	}
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
}

bool SecondOrderSystemGraph::examplesMenu() {
	if (ImGui::MenuItem("symmetrical saddle node")) {
		plotCompiler.setFormulaInput(xFormulaInput, "x");
		plotCompiler.setFormulaInput(yFormulaInput, "-y");
		formulaType = FormulaType::GENERAL;
		return true;
	}
	if (ImGui::MenuItem("symmetrical saddle node")) {
		plotCompiler.setFormulaInput(xFormulaInput, "x");
		plotCompiler.setFormulaInput(yFormulaInput, "-y");
		formulaType = FormulaType::GENERAL;
		return true;
	}
	if (ImGui::MenuItem("damped harmonic oscillator")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.setFormulaInput(xFormulaInput, "y");
		plotCompiler.setFormulaInput(yFormulaInput, "-x-ay");
		formulaType = FormulaType::GENERAL;
		return true;
	}
	if (ImGui::MenuItem("pendulum")) {
		plotCompiler.setFormulaInput(xFormulaInput, "y");
		plotCompiler.setFormulaInput(yFormulaInput, "-sin(x)");
		formulaType = FormulaType::GENERAL;
		return true;
	}
	if (ImGui::MenuItem("van der Pol oscillator")) {
		plotCompiler.setFormulaInput(xFormulaInput, "y");
		plotCompiler.setFormulaInput(yFormulaInput, "-x + y(1 - x^2)");
		formulaType = FormulaType::GENERAL;
		return true;
	}
	if (ImGui::MenuItem("dipole fixed point")) {
		// This is just the complex function x^2. You can get weird things when using complex polynomials, because whole lines get mapped to zero, because what the map does it wrap the complex plane around itself multiple times. This result in multiple lines intersecting at a single point.
		// TODO: https://mabotkin.github.io/complex/
		plotCompiler.setFormulaInput(xFormulaInput, "2xy");
		plotCompiler.setFormulaInput(yFormulaInput, "y^2-x^2");
		formulaType = FormulaType::GENERAL;
		return true;
	} 
	if (ImGui::MenuItem("gravitational well")) {
		plotCompiler.setFormulaInput(xFormulaInput, "y");
		plotCompiler.addParameterIfNotExists("d"); // distance between the bodies.
		plotCompiler.addParameterIfNotExists("m1");
		plotCompiler.addParameterIfNotExists("m2");
		plotCompiler.addParameterIfNotExists("G");
		plotCompiler.setFormulaInput(yFormulaInput, "Gm1/x^2 - Gm2/(d-x)^2");
		formulaType = FormulaType::GENERAL;
		return true;
	}
	if (ImGui::MenuItem("two-eyed monster")) {
		plotCompiler.setFormulaInput(xFormulaInput, "y + y^2");
		plotCompiler.setFormulaInput(yFormulaInput, "-(1/2)x + (1/5)y - xy + (6/5)y^2");
		return true;
	}
	if (ImGui::MenuItem("parrot")) {
		plotCompiler.setFormulaInput(xFormulaInput, "y + y^2");
		plotCompiler.setFormulaInput(yFormulaInput, "-x + (1/5)y - xy + (6/5)y^2");
		return true;
	}

	// x' = sin(x)
	// y' = sin(xy)

	return false;
}

void SecondOrderSystemGraph::computeLoopFunctionOnVisibleRegion(const Runtime::LoopFunction& function, LoopFunctionArray& output, i32 stepsX, i32 stepsY) {

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
			variablesBlock[X_VARIABLE_INDEX_IN_BLOCK] = x;
			variablesBlock[Y_VARIABLE_INDEX_IN_BLOCK] = y;
			input.append(variablesBlock);
		}
	}
	output.reset(1);
	output.resizeWithoutCopy(input.blockCount());

	function(input, output);
}

bool SecondOrderSystemGraph::implicitFunctionGraphSettings(ImplicitFunctionGraph& graph) {
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

void SecondOrderSystemGraph::drawImplicitFunctionGraph(
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

void SecondOrderSystemGraph::calculateImplicitFunctionGraph(const Runtime::LoopFunction& function, std::vector<Vec2>& out) {
	const auto limits = ImPlot::GetPlotLimits();

	LoopFunctionArray output(1);
	const auto steps = 200;
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
