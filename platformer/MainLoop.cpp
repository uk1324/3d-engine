#include <platformer/MainLoop.hpp>
#include <framework/Dbg.hpp>
#include <glad/glad.h>
#include <engine/Math/Utils.hpp>
#include <Put.hpp>

// Assumes points sorted.
f32 piecewiseLinearSample(std::span<Vec2> points, f32 x);

MainLoop::MainLoop() 
	: menu(renderer)
	, game(audio, renderer) {

	settings.tryLoadSettings();
	menu.setAudioSettings(settings.settings.audio);
	menu.setControlsSettings(settings.settings.controls);
	settings.tryLoadGameSave();
}

void MainLoop::update() {
	audio.update(settings.settings.audio);

	switch (state) {
		using enum State;
	case MENU:
		menuUpdate();
		break;
	case GAME:
		gameUpdate();
		break;
	case GAME_PAUSED: {
		gamePausedUpdate();
		break;
	}
		
	case TRANSITIONING:
		if (transitioningState.transitionType == TransitionType::MENU_TO_GAME) {
			const auto previousT = transitioningState.t;
			transitioningState.t += dt * 1.0f;
			if (transitioningState.t > 1.0f) {
				state = State::GAME;
			}

			const auto t1 = 0.4f;
			const auto t2 = 0.6f;
			Vec2 points[] = { Vec2(0.0f, 0.0f), Vec2(t1, 1.0f), Vec2(t2, 1.0f), Vec2(1.0f, 0.0f) };
			f32 opacity = piecewiseLinearSample(points, transitioningState.t);

			if (previousT < t1 && transitioningState.t >= t1) {
				game.onTransitionFromMenu(settings.gameSave);
				audio.musicStream.stop();
				audio.musicStream.useFile("./platformer/Assets/sounds/perfect-beauty.ogg");
				audio.musicStream.play();
				audio.musicStream.loop = true;
			}
			if (transitioningState.t >= t1) {
				const auto musicVolumeScale = 0.35f;
				audio.setMusicStreamVolume(std::clamp((1.0f - opacity) * musicVolumeScale, 0.0f, 1.0f));
			}

			if (transitioningState.t < t1) {
				menuUpdate();
			} else if (transitioningState.t > t2) {
				gameUpdate();
			}
			glEnable(GL_BLEND);
			Dbg::drawFilledAabb(renderer.renderer.camera.aabb(), Vec4(Vec3(0.0f), opacity));
			renderer.renderer.update();
			glDisable(GL_BLEND);
		}
		break;
	}
}

void MainLoop::gameUpdate() {
	const auto& controls = settings.settings.controls;
	const GameInput input{
		.left = Input::isKeyHeld(static_cast<KeyCode>(controls.left)),
		.right = Input::isKeyHeld(static_cast<KeyCode>(controls.right)),
		.jump = Input::isKeyHeld(static_cast<KeyCode>(controls.jump)),
		.use = Input::isKeyHeld(static_cast<KeyCode>(controls.activate))
	};
	game.update(input, controls);
	for (const auto& event : game.thisFrameEvents) {
		switch (event) {
			using enum Game::Event;
		case PREFORM_GAME_SAVE: {
			if (game.activeRoomIndex.has_value()) {
				settings.gameSave.roomIndex = *game.activeRoomIndex;
				settings.saveGameSave();
			} else {
				ASSERT_NOT_REACHED();
			}
			break;
		}

		case PAUSE:
			state = State::GAME_PAUSED;
			menu.currentScreen = Menu::UiScreen::MAIN;
			game.onPause();
			break;
		}
	}
}

void MainLoop::gamePausedUpdate() {
	game.gameRender(settings.settings.controls);
	glEnable(GL_BLEND);
	Dbg::drawFilledAabb(renderer.renderer.camera.aabb(), Vec4(Vec3(0.0f), 0.5f));
	renderer.renderer.update();
	glDisable(GL_BLEND);
	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		game.onUnpause();
		state = State::GAME;
	}

	handleMenuEvent(menu.updateGamePaused(dt, settings.settings));
}

void MainLoop::menuUpdate() {
	handleMenuEvent(menu.updateMainMenu(dt, settings.settings));
}

void MainLoop::handleMenuEvent(Menu::Event event) {
	switch (event) {
		using enum Menu::Event;
	case NONE: break;
	case TRANSITION_TO_GAME:
		state = State::TRANSITIONING;
		transitioningState.reset(TransitionType::MENU_TO_GAME);
		break;

	case TRANSITON_TO_MAIN_MENU:
		state = State::MENU;
		menu.currentScreen = Menu::UiScreen::MAIN;
		break;

	case RESUME_GAME:
		game.onUnpause();
		state = State::GAME;
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
}

void MainLoop::TransitioningState::reset(TransitionType type) {
	transitionType = type;
	t = 0.0f;
}

void MainLoop::TransitioningState::update(f32 dt) {
	const auto speed = 4.0f;
	t += dt * speed;
}

f32 piecewiseLinearSample(std::span<Vec2> points, f32 x) {
	if (points.size() == 0.0f) {
		return 0.0f;
	}

	if (x < points[0].x) {
		return points[0].y;
	}

	for (i32 i = 0; i < i32(points.size()) - 1; i++) {
		const auto& next = points[i + 1];
		if (x <= next.x) {
			const auto& previous = points[i];
			return lerp(previous.y, next.y, (x - previous.x) / (next.x - previous.x));
		}
	}

	return points.back().y;
}
