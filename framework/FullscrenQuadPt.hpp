#pragma once

#include <framework/Vertex2dPtData.hpp>

static constexpr Vertex2dPt fullscreenQuad2dPtVertices[]{
	{ Vec2(-1.0f, 1.0f), Vec2(0.0f, 1.0f) },
	{ Vec2(1.0f, 1.0f), Vec2(1.0f, 1.0f) },
	{ Vec2(-1.0f, -1.0f), Vec2(0.0f, 0.0f) },
	{ Vec2(1.0f, -1.0f), Vec2(1.0f, 0.0f) },
};
static constexpr u32 fullscreenQuad2dPtIndices[]{ 0, 1, 2, 2, 1, 3 };