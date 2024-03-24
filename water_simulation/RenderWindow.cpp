#include "RenderWindow.hpp"
#include <glad/glad.h>

RenderWindow::RenderWindow() 
	: colorTexture(Texture::generate()) 
	, depthTexture(Texture::generate())
	, fbo(Fbo::generate()) {
	
	onScreenResize(Vec2(1.0f));

	fbo.bind();
	colorTexture.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture.handle(), 0);

	depthTexture.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.handle(), 0);

	Fbo::unbind();
}

void RenderWindow::update(Vec2 windowSize) {
	const auto oldWindowSize = previousWindowSize;
	previousWindowSize = windowSize;
	if (windowSize != oldWindowSize) {
		onScreenResize(windowSize);
	}
}

void RenderWindow::onScreenResize(Vec2 newWindowSize) {
	if (newWindowSize.x == 0 || newWindowSize.y == 0) {
		return;
	}

	colorTexture.bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, newWindowSize.x, newWindowSize.y, 0, GL_RGB, GL_FLOAT, nullptr);

	depthTexture.bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, newWindowSize.x, newWindowSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
}
