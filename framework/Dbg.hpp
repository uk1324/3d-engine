#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <Span.hpp>
#include <vector>

namespace Dbg {
	static constexpr Vec3 DEFAULT_COLOR(1.0f);

	void drawDisk(Vec2 pos, float radius, Vec3 color = DEFAULT_COLOR);
	// Using an optional parameter to signify a default parameter allows creating other functions with non required parameters easily. For example if I were to create 2 versions of drawLine one with and one without an lineWidthParameters then I would need to do the same with drawPolygon. Which would require templates or copying code.
	void drawLine(Vec2 pos, Vec2 end, Vec3 color, std::optional<float> lineWidth = std::nullopt);
	void drawPolygon(Span<const Vec2> vertices, Vec3 color = DEFAULT_COLOR, std::optional<float> lineWidth = std::nullopt);

	void update();

	struct Disk {
		Vec2 pos;
		float radius;
		Vec3 color;
		i32 drawIndex;
	};

	struct Line {
		Vec2 start;
		Vec2 end;
		std::optional<float> width;
		Vec3 color;
		i32 drawIndex;
	};

	//enum class EntityType {
	//	LINE,
	//	DISK
	//};

	extern i32 drawnThingsCount;

	extern std::vector<Disk> disks;
	extern std::vector<Line> lines;
}