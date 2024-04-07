#include <water_simulation/MatrixDemo.hpp>
#include <water_simulation/Eigenvectors.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <engine/Math/Angles.hpp>
#include <imgui/imgui_internal.h>
#include <engine/Math/Utils.hpp>
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Color.hpp>
#include <imgui/implot.h>
#include <water_simulation/PlotUtils.hpp>
#include <Gui.hpp>
#include <glad/glad.h>

MatrixDemo::MatrixDemo() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void MatrixDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	using namespace ImGui;

	auto id = DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoTabBar);

	static bool firstFrame = true;
	if (firstFrame) {
		DockBuilderRemoveNode(id);
		DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

		DockBuilderDockWindow("plot", rightId);
		DockBuilderDockWindow("plot settings", leftId);

		DockBuilderFinish(id);
		firstFrame = false;
	}
	//ImPlot::ShowDemoWindow();
	Begin("plot", nullptr, ImGuiWindowFlags_NoTitleBar);

	const auto eigenvectors = computeEigenvectors(matrix);

	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		ImPlot::PushPlotClipRect();

		auto drawEigenvector = [](const Eigenvector& e, Vec3 realColor, Vec3 imagColor) {
			auto realPart = Vec2(e.eigenvector.x.real(), e.eigenvector.y.real()).normalized();
			auto imaginaryPart = Vec2(e.eigenvector.x.imag(), e.eigenvector.y.imag()).normalized();
			plotAddLine(-realPart, realPart, realColor);
			if (imaginaryPart.x > 0.0f || imaginaryPart.y > 0.0f) {
				plotAddLine(-imaginaryPart, imaginaryPart, imagColor);
			}
		};

		drawEigenvector(eigenvectors[0], Color3::YELLOW, Color3::MAGENTA);
		drawEigenvector(eigenvectors[1], Color3::YELLOW / 2.0f, Color3::MAGENTA / 2.0f);

		plotAddArrowOriginDirection(Vec2(0.01f), matrix(0), Color3::RED, 0.03f);
		plotAddArrowOriginDirection(Vec2(0.01f), matrix(1), Color3::GREEN, 0.03f);
		const auto plotMatrix = [](Mat2& m, Vec3 color) {
			plotAddLine(Vec2(0.0f), m.columns[0], color);
			plotAddLine(Vec2(0.0f), m.columns[1], color);
			const auto corner = m.columns[0] + m.columns[1];
			plotAddLine(m.columns[0], corner, color);
			plotAddLine(m.columns[1], corner, color);
		};

		auto m = matrix;
		auto color = Color3::RED;
		for (int i = 0; i < 5; i++) {
			plotMatrix(m, color);
			m = m * matrix;
			color /= 2.0f;
		}
		
		ImPlot::PopPlotClipRect();
		ImPlot::EndPlot();
	}

	End();

	Begin("plot settings");
	auto row0 = matrix.row0();
	auto row1 = matrix.row1();
	ImGui::InputFloat2("##row0", row0.data());
	ImGui::InputFloat2("##row1", row1.data());
	matrix = Mat2::fromRows(row0, row1);

	ImGui::SliderFloat("m00", &matrix(0, 0), -2.0f, 2.0f);
	ImGui::SliderFloat("m10", &matrix(1, 0), -2.0f, 2.0f);
	ImGui::SliderFloat("m01", &matrix(0, 1), -2.0f, 2.0f);
	ImGui::SliderFloat("m11", &matrix(1, 1), -2.0f, 2.0f);

	static float angleMin = 0.0f;
	static float angleMax = TAU<float>;
	ImGui::SliderFloat("angle min", &angleMin, 0.0f, TAU<float>);
	ImGui::SliderFloat("angle max", &angleMax, 0.0f, TAU<float>);

	auto polarInput = [&](const char* label, Vec2& v) {
		auto vAngle = v.angle();
		auto vLength = v.length();
		ImGui::PushID(label);
		if (ImGui::SliderFloat("##angle", &vAngle, angleMin, angleMax) |
			ImGui::SliderFloat("##length", &vLength, 0.0f, 2.0f)) {
			v = Vec2::fromPolar(vAngle, vLength);
		}
		ImGui::PopID();
	};
	polarInput("column0", matrix(0));
	polarInput("column1", matrix(1));
	//Gui::put("v0", eigenvectors.)
	//plotAddLine(Vec   2())
	/*std::vector*/
	//ImPlot::PlotLine()
	/*auto drawEigenspace = []() {

	};*/
	ImGui::Checkbox("lock01", &lock01);
	if (lock01) {
		const auto a = matrix(0, 0), b = matrix(1, 0), c = matrix(0, 1), d = matrix(1, 1);
		matrix(0, 1) = (2.0f * a * d - a * a - d * d) / (4.0f * b);
		//matrix(0, 1) = ((a - d) * (a - d)) / -4 * b;
	}
	End();
}