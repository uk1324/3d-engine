#pragma once

#include <engine/Graphics/Texture.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Vbo.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <Array2d.hpp>

struct MainLoop {
	static MainLoop make();

	static constexpr Vec2T<i64> GRID_SIZE{ 200, 100 };
	Array2d<float> array;

	Texture texture;

	Vbo instancesVbo;

	ShaderProgram& texturedQuadShader;
	Vbo texturedQuadVbo;
	Ibo texturedQuadIbo;
	Vao texturedQuadVao;


	void update();
};