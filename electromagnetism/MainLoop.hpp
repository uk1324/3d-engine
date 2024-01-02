#pragma once

#include <framework/Camera.hpp>
#include <framework/Renderer2d.hpp>
#include <Array2d.hpp>
#include <vector>

struct MainLoop {
	MainLoop();

	void update();

	//Camera camera;
	//int frame = 409;
	//int frame = 1453 * 4;
	//int frame = 1453 * 2;
	int frame = 0;
	bool drawImage = false;
	bool paused = true;
	bool pixelPerfect = false;
	bool connectDiagonals = false;
	//std::vector<std::vector<Vec2>> vertices;
	
	Image32 texture;
	Array2d<float> textureFloat;
	Renderer2d renderer;
	Array2d<float> u;
	//Array2d<float> u_t;
	Array2d<double> u_t;
};