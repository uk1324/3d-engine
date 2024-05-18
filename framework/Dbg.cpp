#include "Dbg.hpp"
#include <engine/Math/Triangle.hpp>

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

void Dbg::drawPolyline(Span<const Vec2> vertices, Vec3 color, std::optional<float> lineWidth) {
	if (vertices.size() < 2) {
		return;
	}
	for (i32 i = 1; i < vertices.size(); i++) {
		Dbg::drawLine(vertices[i - 1], vertices[i], color, lineWidth);
	}
}

void Dbg::drawAabb(Vec2 min, Vec2 max, Vec3 color, std::optional<float> lineWidth) {
	Vec2 vertices[] = {
		min, Vec2(min.x, max.y), max, Vec2(max.x, min.y)
	};
	drawPolygon(vertices, color, lineWidth);
}

void Dbg::drawAabb(const Aabb& aabb, Vec3 color, std::optional<float> lineWidth) {
	drawAabb(aabb.min, aabb.max, color, lineWidth);
}

void Dbg::drawFilledAabb(Vec2 min, Vec2 max, Vec3 color) {
	filledAabbs.push_back(FilledAabb{ .color = color, .min = min, .max = max });
	/*Vec2 v[] = {
		min, Vec2(max.x, min.y), max, Vec2(min.x, max.y)
	};
	filledTriangles.push_back(FilledTriangle{ .v = { v[0], v[1], v[2] }, .color = color });
	filledTriangles.push_back(FilledTriangle{ .v = { v[0], v[2], v[3] }, .color = color });*/
}

void Dbg::drawFilledTriangle(Vec2 v0, Vec2 v1, Vec2 v3, Vec3 color) {
	//triangle
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
std::vector<Dbg::FilledTriangle> Dbg::filledTriangles;
std::vector<Dbg::FilledAabb> Dbg::filledAabbs;

//std::vector<Dbg::Entity> Dbg::entityDrawOrder;