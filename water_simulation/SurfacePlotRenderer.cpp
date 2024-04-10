#include <water_simulation/SurfacePlotRenderer.hpp>
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

SurfacePlotRenderer SurfacePlotRenderer::make() {
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
	//Window::disableCursor();
	return SurfacePlotRenderer{
		.instancesVbo = std::move(instancesVbo),
		.basicShadingShader = ShaderProgram::compile(PLOT_SHADER_SHADER_VERT_PATH, PLOT_SHADER_SHADER_FRAG_PATH),
		.colorMap1d = scientificColorMapTexture(),
		MOVE(graphVbo),
		MOVE(graphIbo),
		MOVE(graphVao),
		MOVE(graphTexture),
		MOVE(graphMesh),
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

void SurfacePlotRenderer::render(const Camera3d& camera, float aspectRatio, Span2d<const float> heightmap, const SurfacePlotSettings& settings) {

	graphTexture.bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SAMPLES_PER_SIDE, SAMPLES_PER_SIDE, GL_RED, GL_FLOAT, heightmap.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	PlotShaderFragUniforms fragUniforms{
		.cameraWorldPosition = camera.position
	};
	shaderSetUniforms(basicShadingShader, fragUniforms);

	basicShadingShader.use();
	basicShadingShader.setTexture("heightmap", 0, graphTexture);
	basicShadingShader.setTexture("colormap", 1, colorMap1d, GL_TEXTURE_1D);
	graph2dInstances.clear();
	addGraph(camera, aspectRatio, heightmap, settings);
	drawInstances(graphVao, instancesVbo, graph2dInstances, [this](usize count) {
		glDrawElementsInstanced(GL_TRIANGLES, graphMesh.indices.size(), GL_UNSIGNED_INT, nullptr, count);
	});
}

void SurfacePlotRenderer::addGraph(const Camera3d& camera, float aspectRatio, Span2d<const float> heightmap, const SurfacePlotSettings& settings) {
	const auto view = camera.viewMatrix();
	const auto projection = Mat4::perspective(degToRad(90.0f), aspectRatio, 0.1f, 1000.0f);

	const auto graphSideLength = LAYER_COUNT * BLOCK_SIZE;
	const auto to01Scale = 1.0f / graphSideLength;
	const auto to01 = Mat4(Mat3::scale(Vec3(to01Scale, 1.0f, to01Scale)));

	const auto toMinusHalfToHalf = Mat4::translation(Vec3(-0.5f, 0.0f, -0.5f)) * to01;

	const auto range = settings.graphMax - settings.graphMin;
	const auto toRange = Mat4(Mat3::scale(Vec3(range.x, 1.0f, range.y))) * toMinusHalfToHalf;

	const auto toScaledRange = Mat4(Mat3::scale(settings.graphScale)) * toRange;

	const auto model = toScaledRange;

	const auto transform = projection * view * model;

	Vec3 shaderScale = Vec3(settings.graphScale.x * range.x, settings.graphScale.y, settings.graphScale.z * range.y);
	const auto minValue = std::ranges::min_element(heightmap.span());
	const auto maxValue = std::ranges::max_element(heightmap.span());
	graph2dInstances.push_back(PlotShaderInstance{ 
		.transform = transform,
		.model = model,
		.scale = shaderScale,
		.samplingScale = Vec2(to01Scale),
		.colormapMin = *minValue,
		.colormapMax = *maxValue,
		.rangeScale = settings.graphScale,
		// Translation due to centering.
		.rangeTranslation = (settings.graphMin + settings.graphMax) / 2.0
	});
}

void SurfacePlotSettings::gui() {
	ImGui::SliderFloat3("scale", graphScale.data(), 0.1f, 3.0f);
	ImGui::SliderFloat2("min", graphMin.data(), -10.0f, 10.0f);
	ImGui::SliderFloat2("max", graphMax.data(), -10.0f, 10.0f);
}
