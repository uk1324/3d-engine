#pragma once

#include <engine/Graphics/Texture.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Vbo.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <water_simulation/Fluid.hpp>
#include <Array2d.hpp>
#include <framework/ImageRenderer.hpp>

struct MainLoop {
	static MainLoop make();

	/*Texture texture;

	std::vector<float> imageData;

	EulerianFluid fluid;*/
	float dt = 1.0f / 60.0f;
	EulerianFluid fluid;
	Array2d<float> smoke;
	Image32 image;
	//Array2d<float> r, g, b;
	ImageRenderer renderer;

	void update();
};