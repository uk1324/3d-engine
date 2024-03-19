#include "GraphDemo.hpp"
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
	: plotter(FunctionPlotter2d::make()) {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void GraphDemo::update() {
	/*plotter.update();

	return;*/
	glClear(GL_COLOR_BUFFER_BIT);

	switch (SettingsManager::settings.activePlotType) {
		using enum SettingsPlotType;

	case FIRST_ORDER_SYSTEM:
		firstOrderSystem.update();
		break;

	case SECOND_ORDER_SYSTEM:
		secondOrderSystem.update();
		break;
	}
	bool plotTypeModified = false;

	// OpenPopup doesn't work inside MainMenuBar.
	// https://github.com/ocornut/imgui/issues/331
	bool openHelpWindow = false;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("plot type")) {
			if (plotTypeModified = ImGui::MenuItem("first order system")) {
				SettingsManager::settings.activePlotType = SettingsPlotType::FIRST_ORDER_SYSTEM;
			} else if (plotTypeModified = ImGui::MenuItem("second order system")) {
				SettingsManager::settings.activePlotType = SettingsPlotType::SECOND_ORDER_SYSTEM;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("examples")) {
			if (ImGui::BeginMenu("first order")) {
				if (plotTypeModified = firstOrderSystem.examplesMenu()) {
					SettingsManager::settings.activePlotType = SettingsPlotType::FIRST_ORDER_SYSTEM;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("second order")) {
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