#pragma once

#include <imgui/implot.h>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <span>

template<typename Function>
void graph(const char* name, Function f, double minX = -1.0, double maxX = -1.0, i32 sampleCount = -1);

//template<typename Function>
//void graph(const char* name, Function f, double minX, double maxX, i32 sampleCount) {
//	{
//
//		const auto plotRect = ImPlot::GetPlotLimits();
//		double min, max;
//		if (minX == -1.0f || maxX == -1.0f) {
//			min = plotRect.X.Min;
//			max = plotRect.X.Max;
//		}
//		else {
//			auto clamp = [&](double x) {
//				return std::clamp(x, plotRect.X.Min, plotRect.X.Max);
//				};
//			min = clamp(minX);
//			max = clamp(maxX);
//			// TODO: What when min >= max?
//			ASSERT(min >= max);
//		}
//
//		std::vector<double> xs;
//		std::vector<double> ys;
//		if (sampleCount == -1) {
//			const auto defaultSampleCount = 300;
//			sampleCount = defaultSampleCount;
//		}
//
//		const double step = (max - min) / double(sampleCount - 1);
//		for (i32 i = 0; i <= sampleCount - 1; i++) {
//			const double t = double(i) / double(sampleCount - 1);
//			const double x = lerp(min, max, t);
//			const double y = f(x);
//			xs.push_back(x);
//			ys.push_back(y);
//		}
//		ImPlot::PlotLine(name, xs.data(), ys.data(), xs.size());
//	}
//}

void plotVec2LineSegments(const char* label, std::span<const Vec2> segmentEndpoints);
void plotAddLine(Vec2 start, Vec2 end, Vec3 color);
void plotAddLine(Vec2 start, Vec2 end, u32 color);
ImU32 plotColorToColorInt(Vec3 color);
//u32 intColor(Vec2 start, Vec2 end, Vec3 color);
//u32 intColor(Vec2 start, Vec2 end, Vec3 color);
