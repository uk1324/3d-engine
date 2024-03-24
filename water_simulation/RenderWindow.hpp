#pragma once

#include <engine/Graphics/Fbo.hpp>
#include <engine/Graphics/Texture.hpp>

struct RenderWindow {
	RenderWindow();

	void update(Vec2 windowSize);
	void onScreenResize(Vec2 newWindowSize);

	Vec2 previousWindowSize = Vec2(-1.0f);

	Texture colorTexture;
	Texture depthTexture;
	Fbo fbo;
};