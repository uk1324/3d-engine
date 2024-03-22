#include <water_simulation/SecondOrderSystemGraph.hpp>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/Color.hpp>
#include <water_simulation/PlotUtils.hpp>
#include <engine/Input/Input.hpp>
#include <Gui.hpp>
#include <Array2d.hpp>
#include <iomanip>

// TODO: Could graph the potentials of conservative/irrotational fields.

// TODO: Could allow intersecting a sufrace with a plane. The result would just be the sum of the intersection with each triangle.

static constexpr auto X_VARIABLE_INDEX_IN_BLOCK = 0;
static constexpr auto Y_VARIABLE_INDEX_IN_BLOCK = 1;

SecondOrderSystemGraph::SecondOrderSystemGraph() {
	plotCompiler.formulaInputs.push_back(&xFormulaInput);
	plotCompiler.formulaInputs.push_back(&yFormulaInput);
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

void SecondOrderSystemGraph::derivativePlot() {
	if (!ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		return;
	}

	ImPlot::SetupAxis(ImAxis_X1, "x");
	ImPlot::SetupAxis(ImAxis_Y1, "y");

	if (!xFormulaInput.loopFunction.has_value() || !yFormulaInput.loopFunction.has_value()) {
		ImPlot::EndPlot();
		return;
	}

	ImPlot::PushStyleColor(ImPlotCol_PlotBg, { 0.5f, 0.5f, 0.5f, 1.0f });
	
	const auto plotRect = ImPlot::GetPlotLimits();
	// TODO: To find the fixed points could compute the abs() of all the values then find the local minima. If the minima are near zero then I could run root finding.

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
			"testPoints",
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

	ImPlot::PopStyleColor();

	ImPlot::EndPlot();
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
	plotCompiler.settingsWindowContent();

	ImGui::Checkbox("paused", &paused);
}

bool SecondOrderSystemGraph::examplesMenu() {
	//if (ImGui::MenuItem("symmetrical unstable node")) {
	//	plotCompiler.setFormulaInput(xFormulaInput, "x");
	//	plotCompiler.setFormulaInput(yFormulaInput, "y");
	//	return true;
	//}
	//if (ImGui::MenuItem("symmetrical stable node")) {
	//	plotCompiler.setFormulaInput(xFormulaInput, "-x");
	//	plotCompiler.setFormulaInput(yFormulaInput, "-y");
	//	return true;
	//}
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
	//if (ImGui::MenuItem("two-eyed monster")) {
	//	plotCompiler.setFormulaInput(xFormulaInput, "y + y^2");
	//	plotCompiler.setFormulaInput(yFormulaInput, "-(1/2)x + (1/5)y - xy + (6/5)y^2");
	//	return true;
	//}
	//if (ImGui::MenuItem("parrot")) {
	//	plotCompiler.setFormulaInput(xFormulaInput, "y + y^2");
	//	plotCompiler.setFormulaInput(yFormulaInput, "-x + (1/5)y - xy + (6/5)y^2");
	//	return true;
	//}

	// x' = sin(x)
	// y' = sin(xy)

	return false;
}
