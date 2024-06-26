#include "ShapeRenderer2d.hpp"
#include "ShaderManager.hpp"
#include "StructUtils.hpp"
#include "FullscrenQuadPt.hpp"
#include <framework/Instancing.hpp>
#include <framework/Shaders/filledTriangleData.hpp>

ShapeRenderer2d ShapeRenderer2d::make(Vbo& fullscreenQuad2dPtVerticesVbo, Ibo& fullscreenQuad2dPtVerticesIbo, Vbo& instancesVbo) {
	Vao filledTriangleVao = Vao::generate();
	// Store the vertices inside the instancesVbo.
	FilledTriangleShader::addAttributesToVao(filledTriangleVao, instancesVbo, instancesVbo);

	return ShapeRenderer2d{
		.diskVao = createInstancingVao<DiskShader>(fullscreenQuad2dPtVerticesVbo, fullscreenQuad2dPtVerticesIbo, instancesVbo),
		.diskShader = MAKE_GENERATED_SHADER(DISK),
		.circleVao = createInstancingVao<CircleShader>(fullscreenQuad2dPtVerticesVbo, fullscreenQuad2dPtVerticesIbo, instancesVbo),
		.circleShader = MAKE_GENERATED_SHADER(CIRCLE),
		.lineVao = createInstancingVao<LineShader>(fullscreenQuad2dPtVerticesVbo, fullscreenQuad2dPtVerticesIbo, instancesVbo),
		.lineShader = MAKE_GENERATED_SHADER(LINE),
		.filledAabbVao = createInstancingVao<FilledAabbShader>(fullscreenQuad2dPtVerticesVbo, fullscreenQuad2dPtVerticesIbo, instancesVbo),
		.filledAabbShader = MAKE_GENERATED_SHADER(FILLED_AABB),
		MOVE(filledTriangleVao),
		.filledTriangleShader = MAKE_GENERATED_SHADER(FILLED_TRIANGLE),

	};
}

void drawFullscreenQuad(usize instanceCount) {
	glDrawElementsInstanced(GL_TRIANGLES, std::size(fullscreenQuad2dPtIndices), GL_UNSIGNED_INT, nullptr, instanceCount);
}

void ShapeRenderer2d::update(Vbo& instancesVbo) {
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	diskShader.use();
	drawInstances(diskVao, instancesVbo, diskInstances, drawFullscreenQuad);
	diskInstances.clear();

	circleShader.use();
	drawInstances(circleVao, instancesVbo, circleInstances, drawFullscreenQuad);
	circleInstances.clear();

	lineShader.use();
	drawInstances(lineVao, instancesVbo, lineInstances, drawFullscreenQuad);
	lineInstances.clear();

	//filledAabbShader.use();
	//drawInstances(filledAabbVao, instancesVbo, filledAabbInstances, drawFullscreenQuad);
	//filledAabbInstances.clear();

	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}
