#pragma once
#include <platformer/Game.hpp>
#include <platformer/Menu.hpp>
#include <platformer/SettingsManager.hpp>

struct MainLoop {
	MainLoop();
	void update();
	void gameUpdate();
	void gamePausedUpdate();
	void menuUpdate();

	void handleMenuEvent(Menu::Event event);

	Game game;
	Menu menu;
	SettingsManager settings;
	GameAudio audio;
	GameRenderer renderer;

	enum class State {
		MENU,
		GAME,
		GAME_PAUSED,
		TRANSITIONING
	};
	State state = State::MENU;

	enum class TransitionType {
		MENU_TO_GAME
	};

	struct TransitioningState {
		TransitionType transitionType = TransitionType::MENU_TO_GAME;
		f32 t = 0.0f;

		void reset(TransitionType type);
		void update(f32 dt);
	} transitioningState;

	f32 dt = 1.0f / 60.0f;
};