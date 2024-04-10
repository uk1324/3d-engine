#pragma once
// Could make a little axis in some corner that moves with the camera. The axis would have text with the names on them. Could use 2d to render it. One issue is that the text would overlap in the wrong order with the axes.

// For movement could allow rotating the camera up and down and around the forward axis maybe using the mouse idk. Could maybe allow rotating the surface instead of the camera.

// To move the camera the window would need to be focused. Then pressing escape could unfocus the window.

/*
Allow specifying the range x min, max and y min max. Could change the names to the values of the plotted parameters.
Store the verticles in layers like this
1 2 5
3 4 6
9 8	7
Then you can choose the amount of vertices per size without weird redindexing and also you can add more vertices easily.

*/

#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <engine/Graphics/Fbo.hpp>
#include <vector>
#include <water_simulation/Camera3d.hpp>
#include <water_simulation/Shaders/plotShaderData.hpp>
#include <water_simulation/IndexedMesh.hpp>
#include <Array2d.hpp>

struct SurfacePlotSettings {
	Vec2 graphMin = Vec2(-3.0f);
	Vec2 graphMax = Vec2(3.0f);
	Vec3 graphScale = Vec3(1.0f);

	void gui();
};

struct SurfacePlotRenderer {

	static SurfacePlotRenderer make();

	Vbo instancesVbo;

	void render(const Camera3d& camera, float aspectRatio, Span2d<const float> heightmap, const SurfacePlotSettings& settings);
	void addGraph(const Camera3d& camera, float aspectRatio, Span2d<const float> heightmap, const SurfacePlotSettings& settings);

	std::vector<PlotShaderInstance> graph2dInstances;

	ShaderProgram basicShadingShader;

	static constexpr float BLOCK_SIZE = 1.0f;
	static constexpr i32 LAYER_COUNT = 100;
	static constexpr i32 VERTICES_PER_SIZE = LAYER_COUNT + 1;

	static constexpr i32 SAMPLES_PER_SIDE = 100;

	Texture colorMap1d;

	Vbo graphVbo;
	Ibo graphIbo;
	Vao graphVao;
	Texture graphTexture;

	IndexedMeshBuilder<PlotShaderVertex> graphMesh;
};