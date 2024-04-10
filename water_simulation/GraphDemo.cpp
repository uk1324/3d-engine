#include "GraphDemo.hpp"
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec4.hpp>
#include <glad/glad.h>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/Utils.hpp>
#include <engine/Math/OdeIntegration/Euler.hpp>
#include <engine/Math/OdeIntegration/Midpoint.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Input/Input.hpp>

// TODO: Allow the compiler to compile 1f32 and 8f32 options
// Does plotting a 3d graph the finding the intersection with zero do anything more than just using marching squares.

GraphDemo::GraphDemo() 
	: plotter(FunctionPlotter2d::make())
	, texture(Texture::generate())
	, renderer2d(Renderer2d::make()) {
	texture.bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}
#include <engine/Math/Aabb.hpp>
#include <Timer.hpp>
#include <Gui.hpp>
struct ProfilingTimes {
	float mainUpdate;
};

void GraphDemo::update() {
	ProfilingTimes profilingTimes;
	//ImGui::ShowDemoWindow();
	//ImPlot::ShowDemoWindow();
	//ImGui::Begin("editor");
	//Aabb sceneWindowWindowSpace = Aabb::fromCorners(
	//	Vec2(ImGui::GetWindowPos()) + ImGui::GetWindowContentRegionMin(),
	//	Vec2(ImGui::GetWindowPos()) + ImGui::GetWindowContentRegionMax()
	//);


	//const auto sceneWindowSize = sceneWindowWindowSpace.size();
	//window.update(sceneWindowSize);
	//window.fbo.bind();
	//glViewport(0, 0, sceneWindowSize.x, sceneWindowSize.y);
	//plotter.update(sceneWindowSize);
	//Fbo::unbind();
	///*ImGui::Image(reinterpret_cast<void*>(window.colorTexture.handle()), sceneWindowSize, Vec2(0.0f, 1.0f), Vec2(1.0f, 0.0f));*/
	//ImGui::Image(reinterpret_cast<void*>(window.colorTexture.handle()), sceneWindowSize, Vec2(0.0f, 1.0f), Vec2(1.0f, 0.0f));
	//ImGui::End();
	////ImGui::CaptureMouseFromApp(false);
	//return;

	glClear(GL_COLOR_BUFFER_BIT);

	Timer mainUpdateTimer;
	switch (SettingsManager::settings.activePlotType) {
		using enum SettingsPlotType;

	case FIRST_ORDER_SYSTEM:
		firstOrderSystem.update();
		break;

	case SECOND_ORDER_SYSTEM:
		secondOrderSystem.update(renderer2d);
		break;
	}
	profilingTimes.mainUpdate = mainUpdateTimer.elapsedMilliseconds();
	bool plotTypeModified = false;

	// OpenPopup doesn't work inside MainMenuBar.
	// https://github.com/ocornut/imgui/issues/331
	bool openHelpWindow = false;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("visualization type")) {
			if (plotTypeModified = ImGui::MenuItem("1d continous system")) {
				SettingsManager::settings.activePlotType = SettingsPlotType::FIRST_ORDER_SYSTEM;
			} else if (plotTypeModified = ImGui::MenuItem("2d continous system")) {
				SettingsManager::settings.activePlotType = SettingsPlotType::SECOND_ORDER_SYSTEM;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("examples")) {
			if (ImGui::BeginMenu("1d continous")) {
				if (plotTypeModified = firstOrderSystem.examplesMenu()) {
					SettingsManager::settings.activePlotType = SettingsPlotType::FIRST_ORDER_SYSTEM;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("2d continous")) {
				if (plotTypeModified = secondOrderSystem.examplesMenu()) {
					SettingsManager::settings.activePlotType = SettingsPlotType::SECOND_ORDER_SYSTEM;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("help")) {
			openHelpWindow = true;
		}
		ImGui::EndMainMenuBar();
	}

	if (plotTypeModified) {
		SettingsManager::settings.saveToFile();
	}

	if (openHelpWindow) {
		GraphDemo::openHelpWindow();
	}
	helpWindow();

	{
		/*ImGui::Begin("profiling");
		Gui::put("main update: %", profilingTimes.mainUpdate);
		ImGui::End();*/
	}
}

static constexpr const char* helpWindowName = "help";

void GraphDemo::openHelpWindow() {
	ImGui::OpenPopup(helpWindowName);
}

void GraphDemo::helpWindow() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 2.0f, -1.0f));
	if (!ImGui::BeginPopupModal(helpWindowName, nullptr)) {
		return;
	}

	ImGui::Text("Controls: ");
	ImGui::Text("Hold ctrl and right click a parameter value to input a new value directly.");

	if (ImGui::Button("close")) {
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}