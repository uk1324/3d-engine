#pragma once

#include <engine/Math/Vec2.hpp>

namespace Window {
	struct Settings {
		int width = 640;
		int height = 480;
		const char* title = "game";
		bool maximized = true;
		bool openGlDebugContext = true;
		int multisamplingSamplesPerPixel = -1; // GLFW_DONT_CARE
	};

	void init(const Settings& settings);
	void terminate();

	Vec2 size();
	float aspectRatio();

	// https://www.glfw.org/docs/3.0/group__clipboard.html
	// "The returned string is valid only until the next call to glfwGetClipboardString or glfwSetClipboardString."
	const char* getClipboard();
	void setClipboard(const char* string);

	void update();
	void close();

	void setTitle(const char* title);

	void setPos(Vec2 pos);
	void setSize(Vec2T<int> size);

	void minimize();

	void enableWindowedFullscreen();

	void disableCursor();
	void enableCursor();
	void toggleCursor();

	bool isCursorEnabled();

	bool resized();

	bool shouldClose();

	void* handle();
};