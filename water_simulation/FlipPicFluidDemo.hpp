#pragma once

#include <water_simulation/FlipPicFluid.hpp>
#include <framework/Renderer2d.hpp>

struct FlipPicFluidDemo {
	FlipPicFluidDemo();

	void update();

	static constexpr float dt = 1.0f / 60.0f;

	Image32 display;
	FlipPicFluid fluid;
	Renderer2d renderer;
};