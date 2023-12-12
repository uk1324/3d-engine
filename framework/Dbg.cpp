#include "Dbg.hpp"

void Dbg::drawDisk(Vec2 pos, float radius, Vec3 color) {
	//entityDrawOrder.push_back(Entity{ EntityType::DISK, static_cast<i32>(disks.size()) });
	disks.push_back(Disk{ .pos = pos, .radius = radius, .color = color });
}

void Dbg::drawCircle(Vec2 pos, float radius, Vec3 color, std::optional<float> lineWidth) {
	circles.push_back(Circle{ pos, radius, lineWidth, color });
}

void Dbg::drawLine(Vec2 pos, Vec2 end, Vec3 color, std::optional<float> lineWidth) {
	//entityDrawOrder.push_back(Entity{ EntityType::LINE, static_cast<i32>(lines.size()) });
	lines.push_back({ pos, end, lineWidth, color });
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
	circles.clear();
	lines.clear();
	//entityDrawOrder.clear();
}

std::vector<Dbg::Disk> Dbg::disks;
std::vector<Dbg::Circle> Dbg::circles;
std::vector<Dbg::Line> Dbg::lines;

//std::vector<Dbg::Entity> Dbg::entityDrawOrder;