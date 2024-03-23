#include <water_simulation/FunctionPlotter2d.hpp>
#include <water_simulation/Shaders/plotShaderData.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Quat.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Math/Utils.hpp>
#include <imgui/imgui.h>
#include <engine/Utils/Timer.hpp>
#include <Dbg.hpp>
#include <vector>

const auto INSTANCE_BUFFER_SIZE = 1024ull * 10ull;

Texture colorMapTexture(std::span<const Vec3> colors) {
	Texture texture = Texture::generate();
	texture.bind(GL_TEXTURE_1D);
	glTexImage1D(
		GL_TEXTURE_1D,
		0,
		GL_RGB,
		colors.size(),
		0,
		GL_RGB,
		GL_FLOAT,
		colors.data()
	);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_1D, 0);
	return texture;
}

Texture scientificColorMapTexture() {
	std::vector<Vec3> colors;
	const auto sampleCount = 100;
	for (i32 i = 0; i < sampleCount; i++) {
		const float t = float(i) / float(sampleCount - 1);
		const auto color = Color3::scientificColoring(t, 0.0f, 1.0f);
		colors.push_back(color);
	}
	return colorMapTexture(colors);
}

FunctionPlotter2d FunctionPlotter2d::make() {
	Vbo instancesVbo(INSTANCE_BUFFER_SIZE);

	auto basicShadingVao = Vao::generate();

	basicShadingVao.bind();
	IndexedMeshBuilder<PlotShaderVertex> graphMesh;
	{
		auto addLayer = [&](i64 layerIndex) {
			const auto xBlocksToAdd = layerIndex;
			{
				const float y0 = float(layerIndex - 1) * BLOCK_SIZE;
				const float y1 = float(layerIndex) * BLOCK_SIZE;
				for (i64 xi = 0; xi < xBlocksToAdd; xi++) {
					const float x0 = float(xi) * BLOCK_SIZE;
					const float x1 = float(xi + 1) * BLOCK_SIZE;
					graphMesh.addQuad(
						PlotShaderVertex{ Vec3(x0, 0.0f, y0) },
						PlotShaderVertex{ Vec3(x1, 0.0f, y0) },
						PlotShaderVertex{ Vec3(x1, 0.0f, y1) },
						PlotShaderVertex{ Vec3(x0, 0.0f, y1) }
					);
				}
			}
			{
 				const float x0 = (xBlocksToAdd - 1) * BLOCK_SIZE;
				const float x1 = (xBlocksToAdd) * BLOCK_SIZE;
				const auto yBlocksToAdd = xBlocksToAdd - 1;
				for (i64 yi = 0; yi < yBlocksToAdd; yi++) {
					const float y0 = float(yi) * BLOCK_SIZE;
					const float y1 = float(yi + 1) * BLOCK_SIZE;
					graphMesh.addQuad(
						PlotShaderVertex{ Vec3(x0, 0.0f, y0) },
						PlotShaderVertex{ Vec3(x1, 0.0f, y0) },
						PlotShaderVertex{ Vec3(x1, 0.0f, y1) },
						PlotShaderVertex{ Vec3(x0, 0.0f, y1) }
					);
				}
			}
		};
		for (int i = 0; i < LAYER_COUNT; i++) {
			addLayer(i + 1);
		}

	}

	auto graphVbo = Vbo(std::span<const PlotShaderVertex>(graphMesh.vertices));
	auto graphIbo = Ibo(graphMesh.indices.data(), graphMesh.indices.size() * sizeof(u32));
	auto graphVao = Vao::generate();
	graphVao.bind();
	graphIbo.bind();
	PlotShaderShader::addAttributesToVao(graphVao, graphVbo, instancesVbo);
	Vao::unbind();
	Ibo::unbind();

	Texture graphTexture = Texture::generate();
	graphTexture.bind();

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_R32F,
		SAMPLES_PER_SIDE,
		SAMPLES_PER_SIDE,
		0,
		GL_RED,
		GL_FLOAT,
		nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
#define MOVE(name) .name = std::move(name)
	Window::disableCursor();
	return FunctionPlotter2d{
		.instancesVbo = std::move(instancesVbo),
		.basicShadingShader = ShaderProgram::compile(PLOT_SHADER_SHADER_VERT_PATH, PLOT_SHADER_SHADER_FRAG_PATH),
		.movementController = {
			.position = Vec3(0.0f),
			.movementSpeed = 5.0f
		},
		.colorMap1d = scientificColorMapTexture(),
		MOVE(graphVbo),
		MOVE(graphIbo),
		MOVE(graphVao),
		MOVE(graphTexture),
		MOVE(graphMesh),
		.array = Array2d<float>(SAMPLES_PER_SIDE, SAMPLES_PER_SIDE),
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

void FunctionPlotter2d::update() {
	if (Input::isKeyDown(KeyCode::T)) {
		Window::toggleCursor();
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, Window::size().x, Window::size().y);
	glEnable(GL_DEPTH_TEST);

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	/*static float range = 1.0f;
	ImGui::SliderFloat("range", &range, 1.0f, 4.0f);
	graphMin = -Vec2(range);
	graphMax = Vec2(range);*/

	ImGui::SliderFloat3("scale", graphScale.data(), 0.1f, 3.0f);

	ImGui::SliderFloat2("min", graphMin.data(), -10.0f, 10.0f);
	ImGui::SliderFloat2("max", graphMax.data(), -10.0f, 10.0f);
	for (i32 xi = 0; xi < SAMPLES_PER_SIDE; xi++) {
		for (i32 yi = 0; yi < SAMPLES_PER_SIDE; yi++) {
			float xt = float(xi) / float(SAMPLES_PER_SIDE - 1);
			float yt = float(yi) / float(SAMPLES_PER_SIDE - 1);
			float x = lerp(graphMin.x, graphMax.x, xt);
			float y = lerp(graphMin.y, graphMax.y, yt);
			float z = 1.5f * sin(x + 0.2f) + cos(y);
			//float z = x*y;
			array(xi, yi) = z;
		}
	}

	graphTexture.bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SAMPLES_PER_SIDE, SAMPLES_PER_SIDE, GL_RED, GL_FLOAT, array.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	const auto dt = 1.0f / 60.0f;
	elapsed += dt;
	if (!Window::isCursorEnabled()) {
		movementController.update(dt);
	} else {
		movementController.lastMousePosition = std::nullopt;
	}
	ImGui::InputFloat3("pos", movementController.position.data());

	PlotShaderFragUniforms fragUniforms{
		.cameraWorldPosition = movementController.position
	};
	shaderSetUniforms(basicShadingShader, fragUniforms);

	basicShadingShader.use();
	basicShadingShader.setTexture("heightmap", 0, graphTexture);
	basicShadingShader.setTexture("colormap", 1, colorMap1d, GL_TEXTURE_1D);
	graph2dInstances.clear();
	drawGraph(array.span2d().asConst(), graphMin, graphMax, graphScale);
	drawInstances(graphVao, instancesVbo, graph2dInstances, [this](usize count) {
		glDrawElementsInstanced(GL_TRIANGLES, graphMesh.indices.size(), GL_UNSIGNED_INT, nullptr, count);
	});
}

void FunctionPlotter2d::drawGraph(Span2d<const float> heightValues, Vec2 rangeMin, Vec2 rangeMax, Vec3 scale) {
	const auto view = movementController.viewMatrix();
	const auto projection = Mat4::perspective(degToRad(90.0f), Window::aspectRatio(), 0.1f, 1000.0f);

	const auto graphSideLength = LAYER_COUNT * BLOCK_SIZE;
	const auto to01Scale = 1.0f / graphSideLength;
	const auto to01 = Mat4(Mat3::scale(Vec3(to01Scale, 1.0f, to01Scale)));

	const auto toMinusHalfToHalf = Mat4::translation(Vec3(-0.5f, 0.0f, -0.5f)) * to01;

	const auto range = rangeMax - rangeMin;
	const auto toRange = Mat4(Mat3::scale(Vec3(range.x, 1.0f, range.y))) * toMinusHalfToHalf;

	const auto toScaledRange = Mat4(Mat3::scale(scale)) * toRange;

	const auto model = toScaledRange;

	const auto transform = projection * view * model;

	Vec3 shaderScale = Vec3(scale.x * range.x, scale.y, scale.z * range.y);

	const auto minValue = std::ranges::min_element(heightValues.span());
	const auto maxValue = std::ranges::max_element(heightValues.span());
	graph2dInstances.push_back(PlotShaderInstance{ 
		.transform = transform,
		.model = model,
		.scale = shaderScale,
		.samplingScale = Vec2(to01Scale),
		.colormapMin = *minValue,
		.colormapMax = *maxValue
	});
}
