#include <game/Renderer.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Quat.hpp>
#include <game/Shaders/infinitePlaneData.hpp>
#include <imgui/imgui.h>
#include <Dbg.hpp>

const auto INSTANCE_BUFFER_SIZE = 1024ull * 10ull;

BasicShadingVertex triangle3dVertices[]{
	{ Vec3(1.0f, 0.0f, 1.0f) },
	{ Vec3(0.0f, 1.0f, 1.0f) },
	{ Vec3(0.0f, 0.0f, 1.0f) }
};

// Uses homogeneous coordinates
// The only difference between trasforming a normal point and a point at infinity is that points at infinity don't get translated, because the w basis in the matrix gets multiplied by zero.
// When using a perspective matrix the w coorindate of the output vector is indepentent of the w coorindate of the input vector. The projection matrix just copies the the z value. So setting w = 0 doesn't cause division by zero.
InfinitePlaneVertex vertices[]{
	// I don't think it's possible to draw an infinite plane using only 2 triangles. I think that might be because the vertices don't get translated so the same thing is rendered no matter the camera position. If you actually try it you will see something like a flickering 1D checkboard. This might be because the view direction is always parallel with the plane.

	// 2 triangle plane
	//{ Vec4(1, 0, 1, 0) },
	//{ Vec4(1, 0, -1, 0) },
	//{ Vec4(-1, 0, 1, 0) },
	//{ Vec4(-1, 0, -1, 0) },

	{ Vec4(0, 0, 0, 1) },
	{ Vec4(1, 0, 0, 0) },
	{ Vec4(0, 0, 1, 0) },
	{ Vec4(-1, 0, 0, 0) },
	{ Vec4(0, 0, -1, 0) },
};

i32 infinitePlaneIndices[]{
	// 2 triangle plane
	/*1, 2, 0,
	1, 3, 2*/

	0, 1, 2,
	0, 2, 3,
	0, 3, 4,
	0, 4, 1
};


Renderer Renderer::make() {
	/*for (auto& b : vertices) {
		b.position *= Mat4(Mat3::rotationX(PI<float>) * Mat3::rotationY(0.0f));
	}*/

	Vbo instancesVbo(INSTANCE_BUFFER_SIZE);
	Vbo triangleVbo(triangle3dVertices, sizeof(triangle3dVertices));

	auto basicShadingVao = Vao::generate();

	basicShadingVao.bind();
	BasicShadingInstances::addAttributesToVao(basicShadingVao, triangleVbo, instancesVbo);

	Vbo infinitePlaneVbo(vertices, sizeof(vertices));
	auto infinitePlaneVao = Vao::generate();
	InfinitePlaneInstances::addAttributesToVao(infinitePlaneVao, infinitePlaneVbo, instancesVbo);
	Ibo infinitePlaneIbo(infinitePlaneIndices, sizeof(infinitePlaneIndices));

	infinitePlaneVao.bind();
	infinitePlaneIbo.bind();
	Vao::unbind();
	Ibo::unbind();

	return Renderer{
		.lastMousePosition = Vec2(0.0f),
		.instancesVbo = std::move(instancesVbo),
		.triangleVbo = std::move(triangleVbo),
		.triangleVao = std::move(basicShadingVao),
		.infinitePlaneVbo = std::move(infinitePlaneVbo),
		.infinitePlaneVao = std::move(infinitePlaneVao),
		.infinitePlaneIbo = std::move(infinitePlaneIbo),
		.infinitePlaneShader = ShaderProgram::compile(INFINITE_PLANE_SHADER_VERT_PATH, INFINITE_PLANE_SHADER_FRAG_PATH),
		.basicShadingShader = ShaderProgram::compile(BASIC_SHADING_SHADER_VERT_PATH, BASIC_SHADING_SHADER_FRAG_PATH),
		.movementController = {
			.position = Vec3(0.0f, 2.0f, 0.0f)
		},
	};
}

template<typename Instance, typename DrawFunction>
static void drawInstances(Vao& vao, Vbo& instancesVbo, const std::vector<Instance>& instances, DrawFunction drawFunction) {
	vao.bind();
	instancesVbo.bind();
	const auto maxInstancesPerDrawCall = INSTANCE_BUFFER_SIZE / sizeof(Instance);
	i32 drawn = 0;
	while (drawn < instances.size()) {
		const auto leftToDraw = instances.size() - drawn;
		const auto toDrawInThisDrawCall = (leftToDraw > maxInstancesPerDrawCall) ? maxInstancesPerDrawCall : leftToDraw;
		boundVboSetData(0, instances.data() + drawn, toDrawInThisDrawCall * sizeof(Instance));
		drawFunction(toDrawInThisDrawCall);
		drawn += toDrawInThisDrawCall;
	}
}

void Renderer::update() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, Window::size().x, Window::size().y);
	glEnable(GL_DEPTH_TEST);

	const auto dt = 1.0f / 60.0f;
	elapsed += dt;
	if (!ImGui::GetIO().WantCaptureMouse) {
		movementController.update(dt);
	}
	//movementController.position = Vec3(0.0f, 2.0f, 0.0f);
	auto target = movementController.position + movementController.cameraForwardRotation() * Vec3::FORWARD;
	const auto view = Mat4::lookAt(movementController.position, target, Vec3::UP);
	const auto projection = Mat4::perspective(degToRad(90.0f), Window::aspectRatio(), 0.1f, 1000.0f);
	const auto viewProjection = view * projection;
	//const auto viewProjection = view;
	//const auto viewProjection = view;

	/*for (auto& v : vertices) {
		v.position *= viewProjection;
		v.position /= v.position.w;
	}*/
	ImGui::InputFloat3("test", movementController.position.data());
	//Gui::put("position %", movementController.position.roundedToDecimalDigits(2));
	Gui::put("look dir %", (target - movementController.position).roundedToDecimalDigits(2));
	//floatin(offset, 0.0f);
	/*const auto model = transformTriangle(Vec3(offset, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 2.0f, 2.0f));*/
	//const auto model = transformTriangle(Vec3(0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

	std::vector<InfinitePlaneInstance> instances;

	infloat(rotationX, 0.0f);
	infloat(rotationY, 0.0f);
	infloat(translationY, 0.0f);
	//rotation += 0.01f;
	const auto model = Mat4(Mat3::rotationX(rotationX) * Mat3::rotationY(rotationY)) * Mat4::translation(Vec3(0.0f, translationY, 0.0f));

	instances.push_back(InfinitePlaneInstance{
		.transform = model * viewProjection,
	});
	infinitePlaneShader.use();

	insliderfloat(detail, 1.0f, 0.0f, 20.0f);

	InfinitePlaneFragUniforms uniforms{
		.inverseViewProjection = viewProjection.inversed(),
		.screenSize = Window::size(),
		.time = elapsed,
		.detail = detail
	};
	shaderSetUniforms(infinitePlaneShader, uniforms);
	drawInstances(infinitePlaneVao, instancesVbo, instances, [](usize instances) { 
		glDrawElementsInstanced(GL_TRIANGLES, std::size(infinitePlaneIndices), GL_UNSIGNED_INT, 0, instances);
	});

	InfinitePlaneVertex v[5];
	memcpy(v, vertices, sizeof(v));

	BasicShadingInstances a;

	/*for (auto& b : v) {
		b.position *= instances.back().transform;
		b.position /= b.position.w;

		a.toDraw.push_back(BasicShadingInstance{
			.transform = Mat4(Mat3::scale(0.2f)) * Mat4::translation(b.position.xyz()) * viewProjection
		});
	}*/

	instances.clear();

	/*for (int x = -10; x < 10; x++) {
		for (int y = -10; y < 10; y++) {
			a.toDraw.push_back(BasicShadingInstance{
				.transform = Mat4::translation(Vec3(x, 0.0f, y)) * viewProjection
			});
		}
	}*/
	/*a.toDraw.push_back(BasicShadingInstance{
		.transform = viewProjection
	});
	a.toDraw.push_back(BasicShadingInstance{
		.transform = Mat4::translation(Vec3(0.0f, 0.0f, 1.0f)) * viewProjection
	});*/
	basicShadingShader.use();
	//drawInstances(instancesVbo, a.toDraw);
	drawInstances(triangleVao, instancesVbo, a.toDraw, [](usize instances) {
		glDrawArraysInstanced(GL_TRIANGLES, 0, std::size(triangle3dVertices), instances);
	});

	a.toDraw.clear();
	
}

Mat4 Renderer::transformTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
	const Mat4 translateBasis{
		v1 - v0,
		v2 - v0,
		Vec4(0.0f),
		Vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
	return translateBasis * Mat4::translation(v0);
}

