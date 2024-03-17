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
#include <water_simulation/Camera3d.hpp>

struct FunctionPlotter2d {
	//FunctionPlotter2d();
	static FunctionPlotter2d make();

	void update();

	Vbo instancesVbo;
	//Vbo triangleVbo;
	//Vao triangleVao;

	/*Vao infiniteLinesVao;
	Vbo infiniteLinesVbo;
	ShaderProgram infinteLinesShader;*/

	Mat4 transformTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2);

	ShaderProgram basicShadingShader;

	Camera3d movementController;


	float elapsed = 0.0f;

	Vbo quadVbo;
	Vao quadVao;

	Vbo graph2dVbo;
	Vao graph2dVao;
};