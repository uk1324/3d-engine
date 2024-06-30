#pragma once

#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <water_simulation/Shaders/line3ShaderData.hpp>

struct LineRenderer {
	static LineRenderer make(Vbo& instancesVbo);
	void addLine(Vec3 start, Vec3 end, Vec3 color);
	void render(const Mat4& viewProjection);

	Vao linesVao;
	Vbo linesVbo;
	ShaderProgram& linesShader;

	std::vector<Vertex3Pc> linesData;
};