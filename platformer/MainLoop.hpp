#pragma once
#include <platformer/Game.hpp>
#include <platformer/Menu.hpp>
#include <platformer/SettingsManager.hpp>

struct MainLoop {
	MainLoop();
	void update();

	Game game;
	Menu menu;
	SettingsManager settings;

	enum class State {
		MENU,
		GAME,
		TRANSITIONING
	};

	f32 dt = 1.0f / 60.0f;
};