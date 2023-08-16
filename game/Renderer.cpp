#include <game/Renderer.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Quat.hpp>
#include <imgui/imgui.h>
#include <Dbg.hpp>

const auto INSTANCE_BUFFER_SIZE = 1024ull * 10ull;

BasicShadingVertex triangle3dVertices[]{
	{ Vec3(1.0f, 0.0f, 1.0f) },
	{ Vec3(0.0f, 1.0f, 1.0f) },
	{ Vec3(0.0f, 0.0f, 1.0f) }
};

Renderer Renderer::make() {
	Vbo instancesVbo(INSTANCE_BUFFER_SIZE);
	Vbo triangleVbo(triangle3dVertices, sizeof(triangle3dVertices));

	auto basicShadingVao = Vao::generate();

	basicShadingVao.bind();
	BasicShadingInstances::addAttributesToVao(basicShadingVao, triangleVbo, instancesVbo);

	return Renderer{
		.lastMousePosition = Vec2(0.0f),
		.instancesVbo = std::move(instancesVbo),
		.triangleVbo = std::move(triangleVbo),
		.triangleVao = std::move(basicShadingVao),
		.basicShadingShader = ShaderProgram::compile(BASIC_SHADING_SHADER_VERT_PATH, BASIC_SHADING_SHADER_FRAG_PATH)
	};
}

void Renderer::update() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, Window::size().x, Window::size().y);
	//glEnable(GL_DEPTH_TEST);

	const auto dt = 1.0f / 60.0f;

	movementController.update(dt);

	auto target = movementController.position + movementController.cameraForwardRotation() * Vec3::FORWARD;
	Mat4 rotation = Mat4::lookAt(movementController.position, target, Vec3::UP);
	Mat4 projection = Mat4::perspective(90.0f, Window::aspectRatio(), 0.1f, 1000.0f);
	Gui::put("position %", movementController.position.roundedToDecimalDigits(2));
	Gui::put("look dir %", (target - movementController.position).roundedToDecimalDigits(2));

	BasicShadingInstances a;
	a.toDraw.push_back(BasicShadingInstance{
		.transform = rotation * projection
	});

	const auto maxInstancesPerDrawCall = INSTANCE_BUFFER_SIZE / sizeof(BasicShadingInstance);
	auto drawn = 0;
	instancesVbo.bind();
	basicShadingShader.use();
	while (drawn < a.toDraw.size()) {
		const auto leftToDraw = a.toDraw.size() - drawn;
		const auto toDrawInThisDrawCall = (leftToDraw > maxInstancesPerDrawCall) ? maxInstancesPerDrawCall : leftToDraw;
		boundVboSetData(0, a.toDraw.data() + drawn, toDrawInThisDrawCall * sizeof(BasicShadingInstance));
		glDrawArraysInstanced(GL_TRIANGLES, 0, std::size(triangle3dVertices), toDrawInThisDrawCall);
		drawn += toDrawInThisDrawCall;
	}
	a.toDraw.clear();
}

