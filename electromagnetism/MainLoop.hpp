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
	bool swappedColors = false;
	bool drawImage = false;
	bool paused = true;
	bool pixelPerfect = false;
	bool connectDiagonals = false;
	std::vector<bool> visited;
	
	Image32 texture;
	Renderer2d renderer;
	Array2d<float> floatTexture;
	Array2d<Vec2> vectorField;
	//DynamicTexture texture;
	std::vector<Vec2> starts;
	std::vector<std::vector<Vec2>> vertices;
	struct Particle {
		Vec2 pos;
		Vec2 vel;
	};
	std::vector<Particle> particles;
	struct Spring {
		Vec2 restPos;
		Vec2 displacementFromRestPos;
		Vec2 vel;
	};
	std::vector<Spring> springs;

};