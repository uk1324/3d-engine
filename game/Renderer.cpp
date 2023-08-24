#include <game/Renderer.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Quat.hpp>
#include <game/Shaders/infinitePlaneData.hpp>
#include <game/Shaders/infiniteLineData.hpp>
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
	{ Vec4(0, 0, 0, 1) },
	{ Vec4(1, 0, 0, 0) },
	{ Vec4(0, 0, 1, 0) },
	{ Vec4(-1, 0, 0, 0) },
	{ Vec4(0, 0, -1, 0) },
};

i32 infinitePlaneIndices[]{
	0, 1, 2,
	0, 2, 3,
	0, 3, 4,
	0, 4, 1
};

// For drawing points could draw a plane that has a constant depth written to the depth buffer.
// Procedural skybox texture.
// Movement on a topological model of the projective plane. Somehow draw curves on it.
// Make a graph from where you can move to where.
// Could use subdivision on the mesh.
// Color each face differently also use depth peeling.
// Maybe it might be better to use the parametric representation.
// Construct a topological model of the projective plane by using a parabolic point.

const usize INFINTE_LINE_VBO_SIZE = 1024 * 40;

std::vector<Vec3> sphereVertices;

void addQuad(std::vector<Vec3>& v, const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& v3) {
	v.push_back(v0);
	v.push_back(v1);
	v.push_back(v2);

	v.push_back(v0);
	v.push_back(v2);
	v.push_back(v3);
}

std::vector<Vec3> calculateFlatShadedNormals(const std::vector<Vec3>& trianglesVertices) {
	std::vector<Vec3> normals;
	ASSERT(trianglesVertices.size() % 3 == 0);
	for (int i = 0; i < trianglesVertices.size(); i += 3) {
		normals.push_back(cross(trianglesVertices[i + 1] - trianglesVertices[i], trianglesVertices[i + 2] - trianglesVertices[i]).normalized());
	}
	return normals;
}

Renderer Renderer::make() {
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

	Vbo infiniteLinesVbo(INFINTE_LINE_VBO_SIZE);
	auto infiniteLinesVao = Vao::generate();
	// TODO: make 2 function one for instances and one for vertex attributes.
	{
		infiniteLinesVao.bind();
		infiniteLinesVao.bind();
		boundVaoSetAttribute(0, ShaderDataType::Float, 4, 0, sizeof(InfiniteLineVertex));
		glVertexAttribDivisor(0, 0);
		Vao::unbind();

	}

	const auto STEPS = 20;
	auto step = 1.0f / STEPS;
	step *= 2.0f;
	/*for (int ix = 0; ix < STEPS; ix++) {
		for (int iy = 0; iy < STEPS; iy++) {*/
	for (int ix = 0; ix < STEPS; ix++) {
		for (int iy = 0; iy < STEPS; iy++) {
			/*float x = ix / static_cast<float>(STEPS);
			float y = iy / static_cast<float>(STEPS);*/
			
			float x = ix / static_cast<float>(STEPS);
			float y = iy / static_cast<float>(STEPS);
			x -= 0.5f;
			x *= 2.0f;
			y -= 0.5f;
			y *= 2.0f;

			Vec3 v0(x, 1.0f, y);
			Vec3 v1(x + step, 1.0f, y);
			Vec3 v2(x + step, 1.0f, y + step);
			Vec3 v3(x, 1.0f, y + step);
			Vec3 n(
				0.0f, 0.0f, static_cast<float>((ix * STEPS + iy)) / (STEPS * STEPS)
			);
			addQuad(sphereVertices, v0.normalized(), v1.normalized(), v2.normalized(), v3.normalized());

			/*float maxX = cos(asin(gy));
			float maxY = sin(acos(gx));*/
			// take out of loop
			/*if (gx > maxX || gy > maxY) {
				continue;
			}*/

			//auto clampX = [&](float x, float y) {
			//	return std::clamp(x, 0.0f, std::max(0.0f, cos(asin(y))));
			//};
			//auto clampY = [&](float x, float y) {
			//	//return y;
			//	return std::clamp(y, 0.0f, std::max(0.0f, sin(acos(clampX(x, y)))));
			//};

			/*auto clampX = [](float x, float y) {
				return x;
				if (Vec2(x, y).length() > 1.0f) {
					auto a = Vec2(x, y).normalized();	
					return a.x;
				}
				return x;
			};
			auto clampY = [](float x, float y) {
				return y;
				if (Vec2(x, y).length() > 1.0f) {
					auto a = Vec2(x, y).normalized();
					return a.y;
				}
				return y;
			};*/

			//auto clampX = [](float x, float y) {
			//	//return x;
			//	if (x > cos(asin(y))) {
			//		return cos(asin(y));
			//		/*auto a = Vec2(x, y).normalized();
			//		return a.x;*/
			//	}
			//	return x;
			//};
			//auto clampY = [&clampX](float x, float y) {
			//	//y = 1.0f - y;
			//	//x = clampX(x, y);
			//	return y;
			//	if (y > sin(acos(x))) {
			//		return sin(acos(x));
			//		/*auto a = Vec2(x, y).normalized();
			//		return a.x;*/
			//	}
			//	return y;
			//};

			/*if (Vec2(gx, gy).length() > 1.0f) {
				auto a = Vec2(gx, gy).normalized();
				continue;
			}

			float x = gx;
			float y = gy;*/

			/*float x = clampX(gx, gy);
			float y = clampY(gx, gy);*/

			/*float x = std::clamp(gx, 0.0f, sin(acos(gy)));
			float y = std::clamp(gx, 0.0f, cos(asin(gx)));*/

			/*float x = gx;
			float y = gy;*/

			/*Vec3 v0(x, cos(asin(x)) * cos(asin(y)), y);
			Vec3 v1(x + step, cos(asin(x + step)) * cos(asin(y)), y);
			Vec3 v2(x + step, cos(asin(x + step)) * cos(asin(y + step)), y + step);
			Vec3 v3(x, cos(asin(x)) * cos(asin(y + step)), y + step);*/

			//Vec3 v0(x, 0.0f, y);
			//Vec3 v1(clampX(x + step, y), 0.0f, y);
			///*Vec3 v2(clampX(x + step, y + step), 0.0f, clampY(x + step, y + step));*/
			//Vec3 v2(x + step, 0.0f, y + step);
			//Vec3 v3(x, 0.0f, clampY(x, y + step));
			 
			/*x = gx;
			y = gy;*/

			//Vec3 v0(x, 0.0f, y);
			//Vec3 v1(x + step, 0.0f, y);
			//Vec3 v2(x + step, 0.0f, y + step);
			//Vec3 v3(x, 0.0f, y + step);
			/*Vec3 n(
				0.0f, 0.0f, static_cast<float>((ix * STEPS + iy)) / (STEPS * STEPS)
			);*/

			/*Vec3 v0(x, 0.0f, y);
			Vec3 v1(x + step / 4.0f, 0.0f, y);
			Vec3 v2(x + step / 4.0f, 0.0f, y + step / 4.0f);
			Vec3 v3(x, 0.0f, y + step / 4.0f);
			Vec3 n(
				0.0f, 0.0f, static_cast<float>((ix * STEPS + iy)) / (STEPS * STEPS)
			);*/

			/*Vec3 n(
				step,
				(v2.y - v0.y) / (sqrt(2.0f) * step),
				step
			);*/
			//addQuad(Vec3(x, cos(asin(x)) * y, y));
			//addQuad(v0, v1, v2, v3, n);
		}
	}

	auto c = sphereVertices;
	sphereVertices.clear();
	sphereVertices.insert(sphereVertices.begin(), c.begin(), c.end());

	for (auto& v : c) {
		v *= Quat(PI<float> / 2.0f, Vec3::RIGHT);
	}
	sphereVertices.insert(sphereVertices.begin(), c.begin(), c.end());
	
	for (auto& v : c) {
		v *= Quat(PI<float> / 2.0f, Vec3::UP);
	}
	sphereVertices.insert(sphereVertices.begin(), c.begin(), c.end());

	c = sphereVertices;

	for (auto& v : c) {
		v = -v;
	}
	/*sphereVertices.insert(sphereVertices.begin(), c.begin(), c.end());*/
	sphereVertices.insert(sphereVertices.begin(), c.rbegin(), c.rend());

	std::vector<BasicShadingVertex> sphereBasicShadingVertices;
	const auto sphereFlatNormals = calculateFlatShadedNormals(sphereVertices);
	for (int i = 0; i < sphereFlatNormals.size(); i++) {
		sphereBasicShadingVertices.push_back(BasicShadingVertex{ .position = sphereVertices[i * 3], .normal = sphereFlatNormals[i] });
		sphereBasicShadingVertices.push_back(BasicShadingVertex{ .position = sphereVertices[i * 3 + 1], .normal = sphereFlatNormals[i] });
		sphereBasicShadingVertices.push_back(BasicShadingVertex{ .position = sphereVertices[i * 3 + 2], .normal = sphereFlatNormals[i] });
	}
 
	Vbo sphereVbo(sphereBasicShadingVertices.data(), sphereBasicShadingVertices.size() * sizeof(decltype(sphereBasicShadingVertices)::value_type));
	auto sphereVao = Vao::generate();
	BasicShadingInstances::addAttributesToVao(sphereVao, sphereVbo, instancesVbo);
	//InfiniteLineInstances::addAttributesToVao(infiniteLinesVao, infiniteLinesVbo, instancesVbo);

#define MOVE(name) .name = std::move(name)

	return Renderer{
		.lastMousePosition = Vec2(0.0f),
		.instancesVbo = std::move(instancesVbo),
		.triangleVbo = std::move(triangleVbo),
		.triangleVao = std::move(basicShadingVao),
		.infinitePlaneVbo = std::move(infinitePlaneVbo),
		.infinitePlaneVao = std::move(infinitePlaneVao),
		.infinitePlaneIbo = std::move(infinitePlaneIbo),
		MOVE(infiniteLinesVao),
		MOVE(infiniteLinesVbo),
		.infinteLinesShader = ShaderProgram::compile(INFINITE_LINE_SHADER_VERT_PATH, INFINITE_LINE_SHADER_FRAG_PATH),
		MOVE(sphereVao),
		MOVE(sphereVbo),
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
	if (!Window::isCursorEnabled()) {
		movementController.update(dt);
	} else {
		movementController.lastMousePosition = std::nullopt;
	}
	infloat(farZ, 20000.0f);
	auto target = movementController.position + movementController.cameraForwardRotation() * Vec3::FORWARD;
	const auto view = Mat4::lookAt(movementController.position, target, Vec3::UP);
	const auto projection = Mat4::perspective(degToRad(90.0f), Window::aspectRatio(), 0.1f, farZ);
	const auto viewProjection = view * projection;

	ImGui::InputFloat3("test", movementController.position.data());
	Gui::put("look dir %", (target - movementController.position).roundedToDecimalDigits(2));


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

	InfinitePlaneFragUniforms uniforms{
		.inverseViewProjection = viewProjection.inversed(),
		.screenSize = Window::size(),
	};
	shaderSetUniforms(infinitePlaneShader, uniforms);
	drawInstances(infinitePlaneVao, instancesVbo, instances, [](usize instances) { 
		glDrawElementsInstanced(GL_TRIANGLES, std::size(infinitePlaneIndices), GL_UNSIGNED_INT, 0, instances);
	});

	BasicShadingInstances a;

	instances.clear();

	
	/*a.toDraw.push_back(BasicShadingInstance{
		.transform = Mat4::translation(Vec3(0.0f, 0.0f, 1.0f)) * viewProjection
	});*/
	basicShadingShader.use();
	drawInstances(triangleVao, instancesVbo, a.toDraw, [](usize instances) {
		glDrawArraysInstanced(GL_TRIANGLES, 0, std::size(triangle3dVertices), instances);
	});

	a.toDraw.clear();

	std::vector<InfiniteLineVertex> vertices;

	auto drawRay = [&vertices](Vec3 start, Vec3 direction) {
		vertices.push_back(InfiniteLineVertex{ Vec4(start, 1.0f) });
		vertices.push_back(InfiniteLineVertex{ Vec4(direction, 0.0f) });
	};

	auto drawLine = [&vertices](Vec3 start, Vec3 end) {
		vertices.push_back(InfiniteLineVertex{ Vec4(start, 1.0f) });
		vertices.push_back(InfiniteLineVertex{ Vec4(end, 1.0f) });
	};

	infinteLinesShader.use();
	shaderSetUniforms(infinteLinesShader, InfiniteLineVertUniforms{ .viewProjection = viewProjection });
	infloat(angle, 0.0f);
	angle += 0.01;
	
	//const auto range = 2.0f;
	// function grapher jit compiler

	insliderfloat(range, 80.0f, 1.0f, 80.0f);
	float x = -range / 2.0f;
	int count = 600;
	float step = range / count;
	auto sample = [](float x) {
		/*return ((sin(2.0f * x) + 1.0f) / 2.0f + 0.01f) * x * x;*/
		/*return ((sin(5.0f * x) + 1.0f) / 2.0f + 1.0f) * x * x;*/
		/*return ((cos(2.0f * x) + 1.0f) / 2.0f + 1.0f) * x * x;*/
		//return pow(x, 6.0f);
		return pow(x, 2.0f);
		//return abs(x);
	};

	/*Vec3 previous = Vec3(x, 0.1f, sample(x));
	for (int i = 0; i <= count; i++) {
		x += step;
		Vec3 current = Vec3(x, 0.1f, sample(x));
		if (i == 0) {
			drawRay(current, Vec3(0.0f, 0.0f, 2.0f));
		} else if (i == count) {
			drawRay(previous, Vec3(0.0f, 0.0f, 2.0f));
		} else {
			drawLine(previous, current);
		}
		previous = current;
	}*/

	std::vector<BasicShadingInstance> sphereInstances;
	sphereInstances.push_back(BasicShadingInstance{
		.transform = Mat4(Mat3::scale(2.0f)) * Mat4::translation(Vec3(0.0f, 5.0f, 0.0f)) * viewProjection
	});

	basicShadingShader.use();
	drawInstances(sphereVao, instancesVbo, sphereInstances, [](usize count) {
		glDrawArraysInstanced(GL_TRIANGLES, 0, sphereVertices.size(), count);
	});
	sphereInstances.clear();

	{
		glDepthFunc(GL_LEQUAL);
		const auto verticesSize = sizeof(InfiniteLineVertex) * vertices.size();
		if (verticesSize > INFINTE_LINE_VBO_SIZE) {
			ASSERT_NOT_REACHED();
		}
		infiniteLinesVbo.setData(0, vertices.data(), verticesSize);
		infiniteLinesVao.bind();
		glDrawArrays(GL_LINES, 0, vertices.size());

		glDepthFunc(GL_LESS);
	}
	infiniteLinesVbo.bind();
	
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

