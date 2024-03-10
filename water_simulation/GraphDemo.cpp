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

void plotVec2s(const char* label, const std::vector<Vec2>&vs) {
	const auto pointsData = reinterpret_cast<const float*>(vs.data());
	ImPlot::PlotScatter(label, pointsData, pointsData + 1, vs.size(), 0, 0, sizeof(float) * 2);
}

void plotLine(const char* label, const std::vector<Vec2>& vs) {
	const auto pointsData = reinterpret_cast<const float*>(vs.data());
	ImPlot::PlotLine(label, pointsData, pointsData + 1, vs.size(), 0, 0, sizeof(float) * 2);
}

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

	using namespace ImGui;

	firstOrderSystem.update();
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

	//	DockBuilderFinish(id);
	//	firstFrame = false;
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