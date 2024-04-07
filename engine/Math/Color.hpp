#pragma once

#include "Vec4.hpp"

namespace Color4 {
	
}

namespace Color3 {
	Vec3 scientificColoring(float v, float minV, float maxV);
	Vec3 fromHsv(float h, float s, float v);
	Vec3 toGrayscale(const Vec3& c);

	static constexpr Vec3 RED(1.0f, 0.0f, 0.0f);
	static constexpr Vec3 GREEN(0.0f, 1.0f, 0.0f);
	static constexpr Vec3 BLUE(0.0f, 0.0f, 1.0f);

	static constexpr Vec3 YELLOW(1.0f, 1.0f, 0.0f);
	static constexpr Vec3 CYAN(0.0f, 1.0f, 1.0f);
	static constexpr Vec3 MAGENTA(1.0f, 0.0f, 1.0f);

	static constexpr Vec3 WHITE(1.0f);
	static constexpr Vec3 BLACK(0.0f);
}