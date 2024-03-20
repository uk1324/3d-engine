#include "PlotUtils.hpp"

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
