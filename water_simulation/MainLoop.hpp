#pragma once

#include <engine/Graphics/Texture.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Vbo.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <water_simulation/Fluid.hpp>
#include <Array2d.hpp>
#include <framework/Renderer2d.hpp>

struct MainLoop {
	static MainLoop make();

	float dt = 1.0f / 60.0f;

	EulerianFluid fluid;
	Array2d<float> smokeR, smokeG, smokeB;
	Image32 image;
	Renderer2d renderer;

	std::vector<Vec2> particles;
	float initialArea = -1.0f;

	void update();
};