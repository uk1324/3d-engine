#include "PlotUtils.hpp"

void plotVec2Scatter(const char* label, std::span<const Vec2> points) {
	const auto pointsData = reinterpret_cast<const float*>(points.data());
	ImPlot::PlotScatter(label, pointsData, pointsData + 1, points.size(), 0, 0, sizeof(Vec2));
}

void plotVec2LineSegments(const char* label, std::span<const Vec2> segmentEndpoints) {
	const auto pointsData = reinterpret_cast<const float*>(segmentEndpoints.data());
	ImPlot::PlotLine(
		label,
		pointsData,
		pointsData + 1,
		segmentEndpoints.size(),
		ImPlotLineFlags_Segments,
		0,
		sizeof(float) * 2
	);
}

//void plotAddLine(Vec2 start, Vec2 end, Vec3 color) {
//
//}

void plotAddLine(Vec2 start, Vec2 end, u32 color) {
	ImPlot::GetPlotDrawList()->AddLine(
		ImPlot::PlotToPixels(ImPlotPoint(start.x, start.y)),
		ImPlot::PlotToPixels(ImPlotPoint(end.x, end.y)),
		color
	);
}

ImU32 plotColorToColorInt(Vec3 color) {
	return ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, 1.0f));
}

void plotAddArrow(Vec2 start, Vec2 end, u32 color, float arrowheadLength) {
	const auto direction = end - start;
	if (direction == Vec2(0.0f)) {
		return;
	}
	const auto angle = direction.angle();
	const auto a = Vec2::oriented(angle + 0.4f);
	const auto b = Vec2::oriented(angle - 0.4f);
	
	plotAddLine(start, end, color);
	plotAddLine(end, end - a.normalized() * arrowheadLength, color);
	plotAddLine(end, end - b.normalized() * arrowheadLength, color);
}

void plotAddArrow(Vec2 start, Vec2 end, Vec3 color, float arrowheadLength) {
	plotAddArrow(start, end, plotColorToColorInt(color), arrowheadLength);
}
