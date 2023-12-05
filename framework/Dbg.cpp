#include "Dbg.hpp"

void Dbg::drawDisk(Vec2 pos, float radius, Vec3 color) {
	disks.push_back(Disk{ .pos = pos, .radius = radius, .color = color, .drawIndex = drawnThingsCount });
	drawnThingsCount++;
}

void Dbg::drawLine(Vec2 pos, Vec2 end, Vec3 color, std::optional<float> lineWidth) {
	lines.push_back({ pos, end, lineWidth, color, drawnThingsCount });
	drawnThingsCount++;
}

void Dbg::drawPolygon(Span<const Vec2> vertices, Vec3 color, std::optional<float> lineWidth) {
	if (vertices.size() < 2) {
		return;
	}
		
	int previous = vertices.size() - 1;
	for (int i = 0; i < vertices.size(); previous = i, i++) {
		Dbg::drawLine(vertices[previous], vertices[i], color, lineWidth);
		previous = i;
	}
}

void Dbg::update() {
	disks.clear();
	lines.clear();
	drawnThingsCount = 0;
}

extern i32 Dbg::drawnThingsCount = 0;

std::vector<Dbg::Disk> Dbg::disks;
std::vector<Dbg::Line> Dbg::lines;