#include <water_simulation/GuiWindowManager.hpp>
#include <imgui/imgui.h>
#include <engine/Math/Aabb.hpp>

void GuiWindowManager::update() {

}

void GuiWindowManager::displayWindow(GuiWindow& window) {
	ImGui::Begin("editor");
	Aabb sceneWindowWindowSpace = Aabb::fromCorners(
		Vec2(ImGui::GetWindowPos()) + ImGui::GetWindowContentRegionMin(),
		Vec2(ImGui::GetWindowPos()) + ImGui::GetWindowContentRegionMax()
	);

	const auto sceneWindowSize = sceneWindowWindowSpace.size();
	/*window.renderWindow.update(sceneWindowSize);
	window.renderWindow.fbo.bind();*/
	//glViewport(0, 0, sceneWindowSize.x, sceneWindowSize.y);
	//plotter.update(sceneWindowSize);
	//Fbo::unbind();
	/*ImGui::Image(reinterpret_cast<void*>(window.colorTexture.handle()), sceneWindowSize, Vec2(0.0f, 1.0f), Vec2(1.0f, 0.0f));*/
	ImGui::Image(
		reinterpret_cast<void*>(window.renderWindow.colorTexture.handle()), 
		sceneWindowSize, 
		Vec2(0.0f, 1.0f), // The coordinates are flipped
		Vec2(1.0f, 0.0f));
	ImGui::End();
}

//GuiWindow& GuiWindowManager::createWindow() {
//	/*windows.push_back(GuiWindow());
//	return windows.back();*/
//}
//
//void GuiWindowManager::freeWindow(GuiWindow& window) {
//	//windows.remove(window);
//}
