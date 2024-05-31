#include <engine/Window.hpp>
#include <engine/Engine.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Log.hpp>
#include <GLFW/glfw3.h>

static GLFWwindow* windowHandle = nullptr;
static bool resizedThisFrame = true;

static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		Input::onKeyDown(key, false);
	} else if (action == GLFW_RELEASE) {
		Input::onKeyUp(key);
	} else if (action == GLFW_REPEAT) {
		Input::onKeyDown(key, true);
	}
}

static void mouseMoveCallback(GLFWwindow* window, double mouseX, double mouseY) {
	Input::onMouseMove(Vec2(static_cast<float>(mouseX), static_cast<float>(mouseY)));
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	// @Hack: Assumes the button ids don't conflinct with keyboard ids.
	if (action == GLFW_PRESS) {
		Input::onKeyDown(button, false);
	} else if (action == GLFW_RELEASE) {
		Input::onKeyUp(button);
	}
}

static void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	Input::onMouseScroll(static_cast<float>(yOffset));
}

static void windowResizeCallback(GLFWwindow* window, int width, int height) {
	resizedThisFrame = true;
}

void Window::init(const Settings& settings) {
	if (settings.maximized) {
		glfwWindowHint(GLFW_MAXIMIZED, true);
	}
	if (settings.openGlDebugContext) {
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}
	glfwWindowHint(GLFW_SAMPLES, settings.multisamplingSamplesPerPixel);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Engine::openGlVersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Engine::openGlVersionMinor);

	windowHandle = glfwCreateWindow(settings.width, settings.height, settings.title, nullptr, nullptr);
	if (windowHandle == nullptr) {
		Log::fatal("failed to create window");
	}
	glfwMakeContextCurrent(windowHandle);

	glfwSetKeyCallback(windowHandle, keyboardCallback);
	glfwSetCursorPosCallback(windowHandle, mouseMoveCallback);
	glfwSetMouseButtonCallback(windowHandle, mouseButtonCallback);
	glfwSetScrollCallback(windowHandle, mouseScrollCallback);
	glfwSetWindowSizeCallback(windowHandle, windowResizeCallback);

	{
		// Prevent cursor from jumping on first mouse move because before that the mouse position is uninitialized.
		double x, y;
		glfwGetCursorPos(windowHandle, &x, &y);
		Input::onMouseMove(Vec2(static_cast<float>(x), static_cast<float>(y)));
	}
}

void Window::terminate() {
	glfwDestroyWindow(windowHandle);
}

Vec2 Window::size() {
	int x, y;
	glfwGetFramebufferSize(windowHandle, &x, &y);
	return Vec2{ static_cast<float>(x), static_cast<float>(y) };
}

float Window::aspectRatio() {
	const auto size = Window::size();
	return size.x / size.y;
}

const char* Window::getClipboard() {
	return glfwGetClipboardString(windowHandle);
}

void Window::setClipboard(const char* string) {
	glfwSetClipboardString(windowHandle, string);
}

void Window::update() {
	resizedThisFrame = false;
	glfwSwapBuffers(windowHandle);
}

void Window::close() {
	glfwSetWindowShouldClose(windowHandle, true);
}

void Window::setTitle(const char* title) {
	glfwSetWindowTitle(windowHandle, title);
}

void Window::setPos(Vec2 pos) {
	glfwSetWindowPos(windowHandle, static_cast<int>(pos.x), static_cast<int>(pos.y));
}

void Window::setSize(Vec2T<int> size) {
	glfwSetWindowSize(windowHandle, size.x, size.y);
}

void Window::minimize() {
	glfwRestoreWindow(windowHandle);
}
void Window::enableWindowedFullscreen() {
	//    if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED))
	//    {
	//        glfwRestoreWindow(engine.window().handle());
	//    }
	//    else
	//    {
	//        glfwMaximizeWindow(engine.window().handle());
	//    }dsfsdfdsffvfdgwdwdwd
	glfwMaximizeWindow(windowHandle);
}

void Window::disableCursor() {
	glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::enableCursor() {
	glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::toggleCursor() {
	const auto mode = glfwGetInputMode(windowHandle, GLFW_CURSOR);
	if (mode == GLFW_CURSOR_NORMAL) {
		disableCursor();
	} else if (mode == GLFW_CURSOR_DISABLED) {
		enableCursor();
	} else {
		// GLFW_CURSOR_HIDDEN what to do?
		ASSERT_NOT_REACHED();
	}
}

bool Window::isCursorEnabled() {
	const auto mode = glfwGetInputMode(windowHandle, GLFW_CURSOR);
	return mode == GLFW_CURSOR_NORMAL;
}

bool Window::resized() {
	return resizedThisFrame;
}

bool Window::shouldClose() {
	return glfwWindowShouldClose(windowHandle);
}

void* Window::handle() {
	return windowHandle;
}