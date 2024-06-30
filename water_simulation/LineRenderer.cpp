#include "LineRenderer.hpp"
#include <engine/Utils/StructUtils.hpp>
#include <framework/ShaderManager.hpp>
#include <glad/glad.h>

static constexpr usize LINE_VBO_CAPACITY = 1000;
static constexpr usize LINE_VBO_SIZE = sizeof(Vertex3Pc) * LINE_VBO_CAPACITY;

LineRenderer LineRenderer::make(Vbo& instancesVbo) {
	Vbo linesVbo(LINE_VBO_SIZE);
	auto linesVao = Vao::generate();
	// TODO: make 2 function one for instances and one for vertex attributes.
	Line3ShaderShader::addAttributesToVao(linesVao, linesVbo, instancesVbo);

	return LineRenderer{
		MOVE(linesVao),
		MOVE(linesVbo),
		.linesShader = MAKE_GENERATED_SHADER(LINE_3_SHADER)
	};
}

void LineRenderer::addLine(Vec3 start, Vec3 end, Vec3 color) {
	linesData.push_back({ .position = start, .color = color });
	linesData.push_back({ .position = end, .color = color });
}

void LineRenderer::render(const Mat4& viewProjection) {
	linesShader.use();
	shaderSetUniforms(linesShader, Line3ShaderVertUniforms{ .viewProjection = viewProjection });

	//glDepthFunc(GL_LEQUAL);
	ASSERT(linesData.size() % 2 == 0);
	auto verticesLeftToSend = linesData.size();
	auto data = linesData.data();
	//const auto toSendInThisBatchByteSize = sizeof(Vertex3Pc) * linesData.size();

	while (verticesLeftToSend > 0) {
		const auto verticesToSendInThisBatchCount = std::min(LINE_VBO_CAPACITY, verticesLeftToSend);
		const auto verticesToSendInThisBatchByteSize = verticesToSendInThisBatchCount * sizeof(Vertex3Pc);
		linesVbo.setData(0, data, verticesToSendInThisBatchByteSize);
		linesVao.bind();
		glLineWidth(5.0f);
		glDrawArrays(GL_LINES, 0, verticesToSendInThisBatchCount);
		verticesLeftToSend -= verticesToSendInThisBatchCount;
		data += verticesToSendInThisBatchCount;
	}
	linesData.clear();


	//if (verticesByteSize > LINE_VBO_SIZE) {
	//	// TODO:
	//	ASSERT_NOT_REACHED();
	//

	//glDepthFunc(GL_LESS);
}
