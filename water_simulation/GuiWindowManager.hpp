#pragma once

#include <string>
#include <water_simulation/RenderWindow.hpp>

struct GuiWindow {
	std::string title;
	RenderWindow3d renderWindow;
};

struct GuiWindowManager {
	
	void update();
	void updateWindow(GuiWindow& window);

};