#include <water_simulation/SecondOrderSystemGraph.hpp>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Input/Input.hpp>

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
	
	const auto plotRect = ImPlot::GetPlotLimits();
	// TODO: To find the fixed points could compute the abs() of all the values then find the local minima. If the minima are near zero then I could run root finding.
	// TODO: Could use NaN to make breaks in lines.

	std::vector<float> inputBlock = plotCompiler.loopFunctionVariablesBlock;
	const auto capacity = 24;
	LoopFunctionArray input(plotCompiler.runtimeVariables.size());
	LoopFunctionArray outputX(1);
	LoopFunctionArray outputY(1);

	const auto minX = i32(floor(plotRect.X.Min / spacing));
	const auto minY = i32(floor(plotRect.Y.Min / spacing));
	const auto maxX = i32(ceil(plotRect.X.Max / spacing));
	const auto maxY = i32(ceil(plotRect.Y.Max / spacing));
	points.clear();

	auto runIntegration = [&]() {
		const auto stepCount = 15;
		outputX.resizeWithoutCopy(input.blockCount());
		outputY.resizeWithoutCopy(input.blockCount());
		for (i32 i = 0; i < stepCount; i++) {
			(*xFormulaInput.loopFunction)(input, outputX);
			(*yFormulaInput.loopFunction)(input, outputY);

			const auto step = spacing / 5;
			for (i32 i = 0; i < input.blockCount(); i++) {
				const Vec2 oldPos(
					input(i, X_VARIABLE_INDEX_IN_BLOCK),
					input(i, Y_VARIABLE_INDEX_IN_BLOCK)
				);
				input(i, X_VARIABLE_INDEX_IN_BLOCK) += outputX(i, 0) * step;
				input(i, Y_VARIABLE_INDEX_IN_BLOCK) += outputY(i, 0) * step;
				const Vec2 newPos(
					input(i, X_VARIABLE_INDEX_IN_BLOCK),
					input(i, Y_VARIABLE_INDEX_IN_BLOCK)
				);
				points.push_back(oldPos);
				points.push_back(newPos);
			}
		}

		for (i32 i = 0; i < input.blockCount(); i++) {
			const Vec2 lastPos(input(i, X_VARIABLE_INDEX_IN_BLOCK), input(i, Y_VARIABLE_INDEX_IN_BLOCK));
			Vec2 derivative(outputX(i, 0), outputY(i, 0));
			// TODO: Could use complex number rotation.
			const auto angle = derivative.angle();
			const auto a = Vec2::oriented(angle + 0.4f);
			const auto b = Vec2::oriented(angle - 0.4f);
			if (derivative.lengthSq() == 0.0f) {
				continue;
			}
			points.push_back(lastPos);
			points.push_back(lastPos - a.normalized() * 0.01f);
			points.push_back(lastPos);
			points.push_back(lastPos - b.normalized() * 0.01f);
		}

		input.clear();
	};

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

	const auto pointsData = reinterpret_cast<float*>(points.data());
	ImPlot::PlotLine("streamlines", pointsData, pointsData + 1, points.size(), ImPlotLineFlags_Segments, 0, sizeof(Vec2));

	{
		const auto testPointsData = reinterpret_cast<const u8*>(testPoints.data());
		ImPlot::PushStyleColor(ImPlotCol_MarkerFill, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
	}

	Input::ignoreImGuiWantCapture = true;
	if (Input::isMouseButtonDown(MouseButton::MIDDLE)) {
		const auto mousePos = ImPlot::GetPlotMousePos();
		const auto p = TestPoint{ Vec2(mousePos.x, mousePos.y) };
		testPoints.push_back(p);
	}
	Input::ignoreImGuiWantCapture = false;

	ImPlot::EndPlot();
}

void SecondOrderSystemGraph::settings() {
	plotCompiler.formulaInputGui("x'=", xFormulaInput);
	plotCompiler.formulaInputGui("y'=", yFormulaInput);
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
		return true;
	}
	if (ImGui::MenuItem("symmetrical saddle node")) {
		plotCompiler.setFormulaInput(xFormulaInput, "x");
		plotCompiler.setFormulaInput(yFormulaInput, "-y");
		return true;
	}
	if (ImGui::MenuItem("damped harmonic oscillator")) {
		plotCompiler.addParameterIfNotExists("a");
		plotCompiler.setFormulaInput(xFormulaInput, "y");
		plotCompiler.setFormulaInput(yFormulaInput, "-x-ay");
		return true;
	}
	if (ImGui::MenuItem("pendulum")) {
		plotCompiler.setFormulaInput(xFormulaInput, "y");
		plotCompiler.setFormulaInput(yFormulaInput, "-sin(x)");
		return true;
	}

	// x' = sin(x)
	// y' = sin(xy)

	return false;
}
