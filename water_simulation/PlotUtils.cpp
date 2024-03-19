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
