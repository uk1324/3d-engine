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

#include <engine/Math/Angles.hpp>

#include <engine/Math/Aabb.hpp>

GraphDemo::GraphDemo() 
	: texture(Texture::generate())
	, renderer2d(Renderer2d::make())
	, lineRenderer(LineRenderer::make(renderer2d.instancesVbo)) {
	texture.bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	const auto loopCount = 15;
	for (int j = 0; j < loopCount; j++) {
		loops.push_back(Loop{});
		float t = float(j) / float(loopCount - 1);
		auto& loop = loops.back();

		i32 steps = 30;
		for (i32 i = 1; i <= steps; i++) {
			const float a0 = TAU<float> *(float(i - 1) / float(steps));
			const float a1 = TAU<float> *(float(i) / float(steps));
			const auto h = lerp(-1.0f, 1.0f, t);
			const auto r = sqrt(1.0f - h * h);
			/*float r = lerp(1.0f, 0.0f, t);
			const auto h = sqrt(1.0f - r * r);*/
			const auto v0 = Vec2::fromPolar(a0, r);
			const auto v1 = Vec2::fromPolar(a1, r);
			loop.points.push_back(Vec3(v0.x, h, v0.y));
			//+Vec3(1.0f)
			//lineRenderer.addLine(Vec3(v0.x, v0.y, 1.0f), Vec3(v1.x, v1.y, 1.0f), Color3::WHITE);
		}
	}

	// add rotated loops
}
#include <Timer.hpp>
#include <Gui.hpp>
struct ProfilingTimes {
	float mainUpdate;
};

#include <engine/Window.hpp>
void GraphDemo::update() {
	/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);*/
	glClear(GL_COLOR_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);

	Window::disableCursor();
	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	if (!paused) {
		for (auto& loop : loops) {
			for (auto& p : loop.points) {
				const auto s = 10.0f;
				const auto r = 28.0f;
				const auto b = 4.0f / 3.0f;
				const auto dt = 1.0f / 600.0f;
				const auto dpdt = Vec3(
					s * (p.y - p.x),
					p.x * (r - p.z) - p.y,
					p.x * p.y - b * p.z
				);
				p += dpdt * dt;
			}
		}
	}
	if (Input::isKeyDown(KeyCode::P)) {
		paused = !paused;
	}

	camera.movementSpeed = 5.0f;

	//lineRenderer.addLine(Vec3(0.0f), Vec3(0.0f, 1.0f, 0.0f), Color3::RED);

	for (auto& loop : loops) {
		std::vector<Vec3> newPoints;
		i32 previousI = loop.points.size() - 1;
		for (i32 i = 0; i < loop.points.size(); i++) {
			const auto current = loop.points[i];
			const auto previous = loop.points[previousI];
			if (current.distanceTo(previous) > 1.0f) {
				const auto midpoint = (current + previous) / 2.0f;
				newPoints.push_back(midpoint);
			}
			newPoints.push_back(current);
			previousI = i;
		}
		loop.points = newPoints;
	}
	//ImGui::Text("%d", int(points.size()));

	/*for (const auto& loop : loops) {*/
	for (i32 j = 0; j < loops.size(); j++) {
		auto& loop = loops[j];
		const auto loopT = float(j) / float(loops.size() - 1);
		i32 previous = loop.points.size() - 1;
		for (i32 i = 0; i < loop.points.size(); i++) {
			const auto t = float(i) / float(loop.points.size());
			const auto c = Color3::fromHsv(t, 1.0f, loopT);
			lineRenderer.addLine(loop.points[i], loop.points[previous], c);
			previous = i;
		}
	}
	const auto perspective = Mat4::perspective(degToRad(90.0f), Window::aspectRatio(), 0.1f, 1000.0f);
	lineRenderer.render(perspective * camera.viewMatrix());
	camera.update(1.0 / 60.0f);

	return;
	ProfilingTimes profilingTimes;

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