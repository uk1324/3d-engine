#include "GraphDemo.hpp"
#include <glad/glad.h>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/Utils.hpp>
#include <engine/Math/OdeIntegration/Euler.hpp>
#include <engine/Math/OdeIntegration/Midpoint.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Input/Input.hpp>

// TODO: Allow the compiler to compile 1f32 and 8f32 options
// Does plotting a 3d graph the finding the intersection with zero do anything more than just using marching squares.
// TODO: Plotting potentials
// If you have a stable fixed point of a first order differential equation then after integration this becomes the minimum of the potential.
GraphDemo::GraphDemo() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

//<<<<<<< HEAD
//template<typename T>
//struct Points2 {
//	std::vector<T> xs;
//	std::vector<T> ys;
//};
//
//template<typename T, typename Function>
//void computeAntiderivative(Points2<T>& out, Function f, T start, T end, i32 steps) {
//	ASSERT(steps % 2 == 0);
//
//	const T step = (end - start) / T(steps);
//	T accumulated = 0.0f;
//
//	out.xs.push_back(start);
//	out.ys.push_back(accumulated);
//
//	// [0, 2], [2, 4]
//	for (i32 i = 0; i < steps; i += 2) {
//		const auto x0 = lerp(start, end, T(i) / T(steps));
//		const auto x1 = lerp(start, end, T(i + 1) / T(steps));
//		const auto x2 = lerp(start, end, T(i + 2) / T(steps));
//
//		// Simpson's rule.
//		const auto integralOnX0toX2 = (step / 3.0f) * (f(x0) + T(4) * f(x1) + f(x2));
//
//		accumulated += integralOnX0toX2;
//		out.xs.push_back(x2);
//		out.ys.push_back(accumulated);
//	}
//}
//
//template<typename T, typename Function>
//void adaptiveIntegrate(Function f, T startT, T endT) {
//	const T initialCondition = 123.0f;
//	const T maxStep = 0.1f;
//	const T minStep = 0.001f;
//	const T tolerance = 0.01f;
//
//	T x = initialCondition;
//	T t = startT;
//	auto step = maxStep;
//	bool flag = true;
//	while (flag) {
//		const T k1 = step * f(t, x);
//		const T k2 = step * f(t + (T(1) / T(4)) * step, x + (T(1) / T(4)) * k1);
//		const T k3 = step * f(t + (T(1) / T(4)))
//	}
//}
//
//template<typename T, typename Function>
//void plotAntiderivative(Function function) {
//	const auto plotRect = ImPlot::GetPlotLimits();
//	Points2<T> points;
//	computeAntiderivative(points, function, 0.0, plotRect.X.Max, 200);
//	ImPlot::PlotLine("antiderivative", points.xs.data(), points.ys.data(), points.xs.size());
//	points.xs.clear();
//	points.ys.clear();
//	computeAntiderivative(points, function, 0.0, plotRect.X.Min, 200);
//	ImPlot::PlotLine("antiderivative", points.xs.data(), points.ys.data(), points.xs.size());
//=======
//void plotVec2s(const char* label, const std::vector<Vec2>&vs) {
//	const auto pointsData = reinterpret_cast<const float*>(vs.data());
//	ImPlot::PlotScatter(label, pointsData, pointsData + 1, vs.size(), 0, 0, sizeof(float) * 2);
//}
//
//void plotLine(const char* label, const std::vector<Vec2>& vs) {
//	const auto pointsData = reinterpret_cast<const float*>(vs.data());
//	ImPlot::PlotLine(label, pointsData, pointsData + 1, vs.size(), 0, 0, sizeof(float) * 2);
//>>>>>>> 99c68c2469a8bf5f3f0ac482674b4d04a9ce940f
//}

template<typename Function>
void graph(const char* name, Function f, double minX = -1.0, double maxX = -1.0, i32 sampleCount = -1) {

	const auto plotRect = ImPlot::GetPlotLimits();
	double min, max;
	if (minX == -1.0f || maxX == -1.0f) {
		min = plotRect.X.Min;
		max = plotRect.X.Max;
	} else {
		auto clamp = [&](double x) {
			return std::clamp(x, plotRect.X.Min, plotRect.X.Max);
		};
		min = clamp(minX);
		max = clamp(maxX);
		// TODO: What when min >= max?
		ASSERT(min >= max);
	}

	std::vector<double> xs;
	std::vector<double> ys;
	// @Performance: Could use a sinle array with double stride.
	if (sampleCount == -1) {
		const auto defaultSampleCount = 300;
		sampleCount = defaultSampleCount;
	}

	const double step = (max - min) / double(sampleCount - 1);
	for (i32 i = 0; i <= sampleCount - 1; i++) {
		const double t = double(i) / double(sampleCount - 1);
		const double x = lerp(min, max, t);
		const double y = f(x);
		xs.push_back(x);
		ys.push_back(y);
	}
	ImPlot::PlotLine(name, xs.data(), ys.data(), xs.size());
}

void GraphDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	// OpenPopup doesn't work inside MainMenuBar.
	// https://github.com/ocornut/imgui/issues/331
	bool openHelpWindow = false;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("plot type")) {
			if (ImGui::MenuItem("first order system")) {
				state = State::FIRST_ORDER_SYSTEM;
			} else if (ImGui::MenuItem("second order system")) {
				state = State::SECOND_ORDER_SYSTEM;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("examples")) {
			if (ImGui::BeginMenu("first order")) {
				if (firstOrderSystem.examplesMenu()) {
					state = State::FIRST_ORDER_SYSTEM;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("second order")) {
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("help")) {
			openHelpWindow = true;
		}
		ImGui::EndMainMenuBar();
	}

	switch (state) {
		using enum State;

	case FIRST_ORDER_SYSTEM:
		firstOrderSystem.update();
		break;

	case SECOND_ORDER_SYSTEM:
		secondOrderSystem.update();
		break;
	}

	if (openHelpWindow) {
		GraphDemo::openHelpWindow();
	}
	helpWindow();
	/*auto id = DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoTabBar);*/
	//auto id = DockSpaceOverViewport(ImGui::GetMainViewport());

	//static bool firstFrame = true;
	//if (firstFrame) {
	//	DockBuilderRemoveNode(id);
	//	DockBuilderAddNode(id);

	//	const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
	//	const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

	//	//DockBuilderDockWindow("plot", rightId);
	//	DockBuilderDockWindow("plot", rightId);
	//	DockBuilderDockWindow("potential plot", rightId);
	//	DockBuilderDockWindow("plot settings", leftId);

//<<<<<<< HEAD
//		DockBuilderFinish(id);
//		firstFrame = false;
//	}
//
//	//ImPlot::ShowDemoWindow();
//
//	Begin("plot", nullptr, ImGuiWindowFlags_NoTitleBar); 
//	/*const auto PI = 3.14;
//	static double xs1[360], ys1[360];
//	for (int i = 0; i < 360; ++i) {
//		double angle = i * 2 * PI / 359.0;
//		xs1[i] = cos(angle); ys1[i] = sin(angle);
//	}*/
//	float xs2[] = { -1,0,1,0,-1 };
//	float ys2[] = { 0,1,0,-1,0 };
//	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
//		ImPlot::SetupAxis(ImAxis_X1);
//		ImPlot::SetupAxis(ImAxis_Y1);
//		//ImPlot::PlotLine("Circle", xs1, ys1, 360);
//		//ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
//		//ImPlot::PlotLine("Diamond", xs2, ys2, 5);
//		auto function = [](double x) { 
//			//return (x - 1.0f) * (x - 2.0) * x;
//			return sin(x);
//		};
//
//		graph("test", function);
//		graph("test", [](double x) { return -cos(x) + 1.0; });
//		plotAntiderivative<double>(function);
//		//plotAntiderivative(function)
//
//
//		ImPlot::EndPlot();
//	}
//	//auto& [xs, ys] = points;
//
//	//if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f)), ImPlotFlags_Equal) {
//	//	/*ImPlot::SetupAxes(nullptr, nullptr);
//	//	ImPlot::SetupAxisScale(ImGuiAxis_X, ImPlotScale_Linear);
//	//	ImPlot::SetupAxisScale(ImGuiAxis_Y, ImPlotScale_Linear);*/
//	//	//ImPlot::SetupAxes(nullptr, nullptr);
//	//	//ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
//	//	//ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);
//	//	//ImPlot::SetupAxis(ImAxis_X2, nullptr, ImPlotAxisFlags_AuxDefault);
//	//	//ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AuxDefault);
//
//	//	graph("test", [](double x) { return sin(x); });
//
//	//	ImPlot::EndPlot();
//=======
//	//	DockBuilderFinish(id);
//	//	firstFrame = false;
//>>>>>>> 99c68c2469a8bf5f3f0ac482674b4d04a9ce940f
	//}

	//firstOrderSystem.update();

	//Begin("plot", nullptr, ImGuiWindowFlags_NoTitleBar);
	//firstOrderSystem.plot();
	//End();

	//Begin("potential plot");
	//End();

	//Begin("plot settings");
	//firstOrderSystem.plotSettings();
	//End();
}

static constexpr const char* helpWindowName = "help";

void GraphDemo::openHelpWindow() {
	ImGui::OpenPopup(helpWindowName);
}

void GraphDemo::helpWindow() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 2.0f, -1.0f));
	if (!ImGui::BeginPopupModal(helpWindowName, nullptr)) {
		return;
	}

	ImGui::Text("Controls: ");
	ImGui::Text("Hold ctrl and right click a parameter value to input a new value directly.");

	if (ImGui::Button("close")) {
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

//void GraphDemo::update() {
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	using namespace ImGui;
//
//	auto id = DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoTabBar);
//
//	static bool firstFrame = true;
//	if (firstFrame) {
//		DockBuilderRemoveNode(id);
//		DockBuilderAddNode(id);
//
//		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
//		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);
//
//		DockBuilderDockWindow("plot", rightId);
//		DockBuilderDockWindow("plot settings", leftId);
//
//		DockBuilderFinish(id);
//		firstFrame = false;
//	}
//
//	Begin("plot", nullptr, ImGuiWindowFlags_NoTitleBar); 
//
//
//	auto calculateDerivative = [this](Vec2 v, float t) -> Vec2 {
//		//return Vec2(x.y, x.x - pow(x.x, 3.0f) - x.y * damping);
//		return Vec2(v.y, sin(v.x));
//		//return Vec2(v.y, sin(v.x) - v.x + 2.0f - v.y * damping);
//		//return Vec2(x.y, pow(x.x, 2.0f) * pow(x.y, 2.0f));
//		//return Vec2(x.y, x.x + x.y + 1.0f);
//		//return Vec2(x.y, x.x - pow(x.x, 3.0f) - x.y);
//		/*return Vec2(v.y, sin(v.x) - v.y * damping);*/
//		//return Vec2(v.y, -v.x);
//		//return Vec2(cos(x.y), sin(x.x));
//	};
//
//	if (!paused) {
//		float dt = 1.0f / 60.0f;
//		for (auto& point : eulerPoints) {
//			point.history.push_back(point.pos);
//			point.pos = eulerStep(calculateDerivative, point.pos, 0.0f, dt);
//		}
//		for (auto& point : midpointPoints) {
//			point.history.push_back(point.pos);
//			point.pos = midpointStep(calculateDerivative, point.pos, 0.0f, dt);
//		}
//		for (auto& point : rungeKutta4Points) {
//			point.history.push_back(point.pos);
//			point.pos = rungeKutta4Step(calculateDerivative, point.pos, 0.0f, dt);
//		}
//	}
//
//	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
//		//ImPlot::SetupAxesLimits(-2.0f, 2.0f, -2.0f, 2.0f);
//		ImPlot::SetupAxis(ImAxis_X1);
//		ImPlot::SetupAxis(ImAxis_Y1);
//
//		const auto plotRect = ImPlot::GetPlotLimits();
//		/*auto spacing = std::min(
//			std::abs(plotRect.X.Max - plotRect.X.Min), 
//			std::abs(plotRect.Y.Max - plotRect.Y.Min));
//		spacing /= 50;*/
//
//		// TODO: To find the fixed points could compute the abs() of all the values then find the local minima. If the minima are near zero then I could run root finding.
//		// TODO: Could use NaN to make breaks in lines.
//
//		const auto minX = i32(floor(plotRect.X.Min / spacing));
//		const auto minY = i32(floor(plotRect.Y.Min / spacing));
//		const auto maxX = i32(ceil(plotRect.X.Max / spacing));
//		const auto maxY = i32(ceil(plotRect.Y.Max / spacing));
//		points.clear();
//		for (i32 xi = minX; xi <= maxX; xi++) {
//			for (i32 yi = minY; yi <= maxY; yi++) {
//				const auto x = xi * spacing;
//				const auto y = yi * spacing;
//
//				//auto pos = Vec2(x, y);
//				//points.push_back(pos);
//				//auto derivative = calculateDerivative(pos);
//				//derivative = derivative.normalized() * (spacing);
//				//points.push_back(pos + derivative);
//
//				auto current = Vec2(x, y);
//				const auto step = spacing / 5;
//				Vec2 derivative;
//				for (int i = 0; i < 15; i++) {
//					derivative = calculateDerivative(current, 0.0f);
//					const auto newPos = current + derivative * step;
//					points.push_back(current);
//					points.push_back(newPos);
//					current = newPos;
//				}
//				const auto angle = derivative.angle();
//				const auto a = Vec2::oriented(angle + 0.2f);
//				const auto b = Vec2::oriented(angle - 0.2f);
//				points.push_back(current);
//				points.push_back(current - a.normalized() * 0.01f);
//				points.push_back(current);
//				points.push_back(current - b.normalized() * 0.01f);
//				//if ()
//				//auto pos = Vec2(x, y);
//				//points.push_back(pos);
//				//auto derivative = calculateDerivative(pos);
//				//derivative = derivative.normalized() * (spacing);
//				//points.push_back(pos + derivative);
//			}
//		}
//		const auto pointsData = reinterpret_cast<float*>(points.data());
//		ImPlot::PlotLine("testa", pointsData, pointsData + 1, points.size(), ImPlotLineFlags_Segments, 0, sizeof(Vec2));
//
//		auto plotScatterTestPoints = [](const std::vector<TestPoint>& testPoints, Vec3 color) {
//			const auto testPointsData = reinterpret_cast<const u8*>(testPoints.data());
//			ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color.x, color.y, color.z, 1.0f));
//			ImPlot::PlotScatter(
//				"testPoints",
//				reinterpret_cast<const float*>(testPointsData + offsetof(TestPoint, pos.x)),
//				reinterpret_cast<const float*>(testPointsData + offsetof(TestPoint, pos.y)),
//				testPoints.size(),
//				0,
//				0,
//				sizeof(TestPoint)
//			);
//			for (const auto& p : testPoints) {
//				plotLine("test", p.history);
//			}
//			ImPlot::PopStyleColor();
//		};
//		plotScatterTestPoints(eulerPoints, Color3::RED);
//		plotScatterTestPoints(midpointPoints, Color3::GREEN);
//		plotScatterTestPoints(rungeKutta4Points, Color3::WHITE);
//		
//
//		if (Input::isKeyDown(KeyCode::T)) {
//			const auto mousePos = ImPlot::GetPlotMousePos();
//			const auto p = TestPoint{ Vec2(mousePos.x, mousePos.y) };
//			eulerPoints.push_back(p);
//			midpointPoints.push_back(p);
//			rungeKutta4Points.push_back(p);
//		}
//		//plotVec2s("testa", points);
//
//		//ImPlot::line
//		//graph("test", [&](double x) { return calculateDerivative(Vec2(x, 0.0f)).y; });
//
//		ImPlot::EndPlot();
//	}
//
//	End();
//
//	Begin("plot settings");
//
//	SliderFloat("spacing", &spacing, 0.005f, 0.25f);
//	SliderFloat("damping", &damping, -1.0f, 10.0f);
//	Checkbox("paused", &paused);
//
//	End();
//}