#include <engine/Engine.hpp>
#include <engine/Log.hpp>
#include <engine/Input/Input.hpp>
#include <Timer.hpp>
#include <Put.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

static void glfwErrorCallback(int errorCode, const char* errorMessage) {
	LOG_FATAL("glfw error % %", errorCode, errorMessage);
}

static void openGlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	// Fragment spriteShader recompiled due to state change.
	if (id == 2)
		return;

	// glLineWidth deprecated.
	if (id == 7)
		return;

	// Read pixels
	// Performance
	// Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
	if (id == 131154)
		return;

	std::string errorMessage = "source: ";
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             errorMessage += "api"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   errorMessage += "window system"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: errorMessage += "shader compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     errorMessage += "third party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     errorMessage += "application"; break;
	case GL_DEBUG_SOURCE_OTHER:           errorMessage += "other"; break;
	}
	errorMessage += '\n';

	errorMessage += " type: ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               errorMessage += "error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: errorMessage += "deprecated behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  errorMessage += "undefined behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         errorMessage += "portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         errorMessage += "performance"; break;
	case GL_DEBUG_TYPE_MARKER:              errorMessage += "marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          errorMessage += "push group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           errorMessage += "pop group"; break;
	case GL_DEBUG_TYPE_OTHER:               errorMessage += "other"; break;
	}
	errorMessage += '\n';

	errorMessage += " severity: ";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         errorMessage += "high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       errorMessage += "medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          errorMessage += "low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: errorMessage += "notification"; break;
	}
	errorMessage += '\n';

	errorMessage += " message: ";
	errorMessage += message;
	errorMessage += '\n';

	if (id == 1286 || severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		LOG_ERROR("OpenGL error %", errorMessage);
	} else {
		LOG_FATAL("OpenGL error %", errorMessage);
	}

	

}

void Engine::initAll(const Window::Settings& windowSettings) {
	Timer timer;
	put("Engine::init start");
	initGlfw();
	{
		Timer timer;
		Window::init(windowSettings);
		put("Window::init took: %", timer.elapsedMilliseconds());
	}
	initOpenGl();
	initImGui();

	put("Engine::init took: %", timer.elapsedMilliseconds());
}

void Engine::terminateAll() {
	terminateImGui();
	Window::terminate();
	terminateGlfw();
}

void Engine::initGlfw() {
	if (glfwInit() == GLFW_FALSE) {
		LOG_FATAL("failed to initialize GLFW");
	}
	glfwSetErrorCallback(glfwErrorCallback);
}

void Engine::terminateGlfw() {
	glfwTerminate();
}

void Engine::initOpenGl() {
	if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == false) {
		LOG_FATAL("failed to initialize OpenGL");
	}

	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openGlErrorCallback, nullptr);
	} else {
		LOG_ERROR("failed to initialize debug output");
	}
}

void Engine::initImGui() {
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Regular.ttf", 20);
	/*io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;*/
	auto& style = ImGui::GetStyle();
	style.WindowRounding = 5.0f;

	ImGui::GetIO().IniFilename = "./cached/imgui.ini";

	ImGui_ImplGlfw_InitForOpenGL(reinterpret_cast<GLFWwindow*>(Window::handle()), true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

void Engine::terminateImGui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Engine::updateFrameStart() {
	Input::update();
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Engine::updateFrameEnd() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	Window::update();
}

int Engine::openGlVersionMajor = 4;
int Engine::openGlVersionMinor = 3;