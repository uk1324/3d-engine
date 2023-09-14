#pragma once
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Vbo.hpp>
#include <vector>
#include <glad/glad.h>

template<typename Instance, typename DrawFunction>
static void drawInstances(Vao& vao, Vbo& instancesVbo, const std::vector<Instance>& instances, DrawFunction drawFunction) {
	vao.bind();
	instancesVbo.bind();

	// @Performance?
	GLint64 instanceBufferSize;
	glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &instanceBufferSize);

	const auto maxInstancesPerDrawCall = instanceBufferSize / sizeof(Instance);
	i32 drawn = 0;
	while (drawn < instances.size()) {
		const auto leftToDraw = instances.size() - drawn;
		const auto toDrawInThisDrawCall = (leftToDraw > maxInstancesPerDrawCall) ? maxInstancesPerDrawCall : leftToDraw;
		boundVboSetData(0, instances.data() + drawn, toDrawInThisDrawCall * sizeof(Instance));
		drawFunction(toDrawInThisDrawCall);
		drawn += toDrawInThisDrawCall;
	}
}