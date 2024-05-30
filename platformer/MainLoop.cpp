#include <platformer/MainLoop.hpp>

MainLoop::MainLoop() 
	: menu(game.renderer) {

	settings.tryLoadSettings();
	menu.setAudioSettings(settings.settings.audio);
	menu.setControlsSettings(settings.settings.controls);
}

void MainLoop::update() {
	const auto event = menu.update(dt);
	switch (event) {
		using enum Menu::Event;
		case NONE: break;
		case TRANSITION_TO_GAME:
			break;

		case SAVE_SOUND_SETTINGS:
			settings.settings.audio = menu.getAudioSettings();
			settings.saveSettings();
			break;

		case SAVE_CONTROLS:
			settings.settings.controls = menu.getControlsSettings();
			settings.saveSettings();
			break;
	}
	//game.update();
}