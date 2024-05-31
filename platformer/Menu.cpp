#include "Menu.hpp"
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <engine/Math/Utils.hpp>
#include <engine/Input/InputUtils.hpp>
#include <engine/Window.hpp>
#include <framework/Dbg.hpp>

// Making a layout by calculating the total height and scaling things so that they fit would make it so that elements (like text) change size in different screens. 
// If things don't fit on a single screen then some sort of scrolling needs to be implemented. The simplest thing is to probably just specify sizes in fractions of full screen size and just don't put too many things.
// The layout builder could return an index into a position array. This index would be stored in the retained structs that would store things like the animationT.

static void addButton(std::vector<Menu::Button>& buttons, UiLayout& layout, SelectionLayout& selectionLayout, std::string_view text, f32 sizeY) {
	buttons.push_back(Menu::Button{
		.id = layout.addBlock(sizeY),
		.selectionId = selectionLayout.add(),
		.text = text,
		.hoverAnimationT = 0.0f,
	});
}

static constexpr const char* playButtonName = "play";
static constexpr const char* soundSettingsButtonName = "sound settings";
static constexpr const char* controlsButtonName = "controls";
static constexpr const char* exitButtonName = "exit";
static constexpr const char* resumeButtonName = "resume";
static constexpr const char* mainMenuButtonName = "main menu";
static constexpr const char* saveButtonName = "save";
static constexpr const char* cancelButtonName = "cancel";
static constexpr const char* masterVolumeSliderName = "master";
static constexpr const char* soundEffectVolumeSliderName = "sound effect";
static constexpr const char* musicVolumeSliderName = "music";

static constexpr f32 buttonSize = 0.05f;
static constexpr f32 smallTitleSize = buttonSize * 1.5f;

Menu::Menu(GameRenderer& renderer)
	: renderer(renderer) {
	camera.zoom /= 280.0f;

	f32 titleSize = 0.15f;

	const auto padding = 0.01f;

	{
		mainMenuUi.titleId = mainMenuUi.layout.addBlock(titleSize);

		std::string_view buttonNames[]{
			playButtonName,
			soundSettingsButtonName,
			controlsButtonName,
			exitButtonName
		};
		for (const auto& name : buttonNames) {
			mainMenuUi.layout.addPadding(padding);
			addButton(mainMenuUi.buttons, mainMenuUi.layout, mainMenuUi.selection, name, buttonSize);
		}
	}

	{
		gamePausedUi.titleId = gamePausedUi.layout.addBlock(smallTitleSize);

		std::string_view buttonNames[]{
			resumeButtonName,
			soundSettingsButtonName,
			controlsButtonName,
			mainMenuButtonName,
			exitButtonName
		};
		for (const auto& name : buttonNames) {
			gamePausedUi.layout.addPadding(padding);
			addButton(gamePausedUi.buttons, gamePausedUi.layout, gamePausedUi.selection, name, buttonSize);
		}
	}

	{
		auto addKeycodeInput = [&](std::string_view name, KeyCode keycode) -> i32 {
			i32 index = controlsUi.keycodeInputs.size();
			controlsUi.keycodeInputs.push_back(KeycodeInput{
				.name = name,
				.id = controlsUi.layout.addBlock(buttonSize),
				.selectionId = controlsUi.selection.add(),
				.keycode = keycode,
			});
			return index;
		};
		controlsUi.titleId = controlsUi.layout.addBlock(smallTitleSize);
		controlsUi.layout.addPadding(padding);
		controlsUi.leftKeycodeInputIndex = addKeycodeInput("left", KeyCode::A);
		controlsUi.layout.addPadding(padding);
		controlsUi.rightKeycodeInputIndex = addKeycodeInput("right", KeyCode::D);
		controlsUi.layout.addPadding(padding);
		controlsUi.jumpKeycodeInputIndex = addKeycodeInput("jump", KeyCode::SPACE);
		controlsUi.layout.addPadding(padding);
		controlsUi.activateKeycodeInputIndex = addKeycodeInput("activate", KeyCode::J);
		controlsUi.layout.addPadding(padding);
		addButton(controlsUi.buttons, controlsUi.layout, controlsUi.selection, saveButtonName, buttonSize);
		controlsUi.layout.addPadding(padding);
		addButton(controlsUi.buttons, controlsUi.layout, controlsUi.selection, cancelButtonName, buttonSize);
	}

	{
		auto addSliderInput = [&](std::string_view name) -> i32 {
			i32 index = soundSettingsUi.sliderInputs.size();
			soundSettingsUi.sliderInputs.push_back(SliderInput{
				.name = name,
				.id = soundSettingsUi.layout.addBlock(buttonSize),
				.selectionId = soundSettingsUi.selection.add(),
			});
			return index;
		};
		soundSettingsUi.titleId = soundSettingsUi.layout.addBlock(smallTitleSize);
		soundSettingsUi.layout.addPadding(padding);

		soundSettingsUi.masterVolumeSliderIndex = addSliderInput(masterVolumeSliderName);
		soundSettingsUi.layout.addPadding(padding);

		soundSettingsUi.soundEffectVolumeSliderIndex = addSliderInput(soundEffectVolumeSliderName);
		soundSettingsUi.layout.addPadding(padding);

		soundSettingsUi.musicVolumeSliderIndex = addSliderInput(musicVolumeSliderName);
		soundSettingsUi.layout.addPadding(padding);

		addButton(soundSettingsUi.buttons, soundSettingsUi.layout, soundSettingsUi.selection, saveButtonName, buttonSize);
		soundSettingsUi.layout.addPadding(padding);

		addButton(soundSettingsUi.buttons, soundSettingsUi.layout, soundSettingsUi.selection, cancelButtonName, buttonSize);
	}
}

void Menu::drawText(std::string_view text, const UiLayout& layout, i32 id, f32 offset) {
	auto& block = layout.blocks[id];
	const auto center = Vec2(camera.pos.x, block.worldCenter());
	//drawTextCentered(text, center, block.worldSize(), offset);
	drawTextCentered(text, center, block.worldSize(), offset);
}

static void updateHoverAnimation(f32& t, bool isSelected, f32 dt) {
	const auto animationSpeed = 6.0f;
	if (isSelected) {
		t += dt * animationSpeed;
	} else {
		t -= dt * animationSpeed;
	}
	t = std::clamp(t, 0.0f, 1.0f);
}

static f32 hoverAnimationTToOffset(f32 animationT) {
	return -animationT * 0.0035f;
}

//Menu::Event Menu::update(f32 dt) {
//	Event result = Event::NONE;
//	switch (currentScreen) {
//		using enum UiScreen;
//	case MAIN:
//		result = updateMainMenuUi(dt);
//		break;
//	case SOUND_SETTINGS:
//		result = updateSoundSettingsUi(dt);
//		break;
//	case CONTROLS:
//		result = updateControlsUi(dt);
//		break;
//	}
//
//	renderUpdate();
//	return result;
//}

Menu::Event Menu::updateMainMenu(f32 dt, const Settings& settings) {
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, Window::size().x, Window::size().y);
	camera.aspectRatio = Window::aspectRatio();
	renderer.renderBackground();
	renderer.update(); // TODO: maybe make renderer.updateTime(); or maybe store local time and pass it to the function

	Event result = Event::NONE;
	switch (currentScreen) {
		using enum UiScreen;
	case MAIN:
		result = updateMainMenuUi(dt, settings);
		break;
	case SOUND_SETTINGS:
		result = updateSoundSettingsUi(dt);
		break;
	case CONTROLS:
		result = updateControlsUi(dt);
		break;
	}

	renderUpdate();
	return result;
}

Menu::Event Menu::updateGamePaused(f32 dt, const Settings& settings) {
	Event result = Event::NONE;
	switch (currentScreen) {
		using enum UiScreen;
	case MAIN:
		result = updateGamePausedUi(dt, settings);
		break;
	case SOUND_SETTINGS:
		result = updateSoundSettingsUi(dt);
		break;
	case CONTROLS:
		result = updateControlsUi(dt);
		break;
	}

	renderUpdate();
	renderer.update();
	return result;
}

void Menu::renderUpdate() {
	renderer.renderer.camera = camera;

	glEnable(GL_BLEND);
	renderer.fontRenderer.render(renderer.font, renderer.renderer.instancesVbo);
	glDisable(GL_BLEND);
}

void Menu::changeToControlsUi(const Settings& settings) {
	currentScreen = UiScreen::CONTROLS;
	setControlsSettings(settings.controls);
}

void Menu::changeToAudioSettingsUi(const Settings& settings) {
	currentScreen = UiScreen::SOUND_SETTINGS;
	setAudioSettings(settings.audio);
}

void Menu::drawTextCentered(std::string_view text, Vec2 position, f32 height, f32 offset) {
	const auto info = renderer.fontRenderer.getTextInfo(renderer.font, height, text);
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	drawText(text, position, height, offset);
	// TODO: Could make addTextToDraw a templated function. It would just set the nescessary fields. Other parameters will be default and will need to be set after the function call.
}

void Menu::drawText(std::string_view text, Vec2 bottomLeftPosition, f32 height, f32 offset) {
	i32 startIndex = renderer.fontRenderer.basicTextInstances.size();
	renderer.fontRenderer.addTextToDraw(
		renderer.font,
		bottomLeftPosition,
		camera.worldToCameraToNdc(),
		height,
		text
	);
	for (i32 i = startIndex; i < renderer.fontRenderer.basicTextInstances.size(); i++) {
		renderer.fontRenderer.basicTextInstances[i].offset = offset;
	}
}

Aabb Menu::getTextAabb(std::string_view text, Vec2 position, f32 height) {
	const auto info = renderer.fontRenderer.getTextInfo(renderer.font, height, text);
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	position.y -= info.size.y / 2.0f;
	return Aabb::fromCorners(position, position + info.size);
}

Menu::Event Menu::updateMainMenuUi(f32 dt, const Settings& settings) {
	Event result = Event::NONE;
	mainMenuUi.layout.update(camera);
	mainMenuUi.selection.update();

	drawText("AXOMETRIC", mainMenuUi.layout, mainMenuUi.titleId, -0.002f);
	for (const auto& button : mainMenuUi.buttons) {
		drawText(button.text, mainMenuUi.layout, button.id, -button.hoverAnimationT * 0.0035f);
	}

	for (auto& button : mainMenuUi.buttons) {
		const auto isSelected = mainMenuUi.selection.isSelected(button.selectionId);
		updateHoverAnimation(button.hoverAnimationT, isSelected, dt);

		const auto pressed = Input::isKeyDown(KeyCode::ENTER);
		if (!isSelected || !pressed) {
			continue;
		}

		if (button.text == playButtonName) {
			result = Event::TRANSITION_TO_GAME;
		} else if (button.text == soundSettingsButtonName) {
			changeToAudioSettingsUi(settings);
		} else if (button.text == controlsButtonName) {
			changeToControlsUi(settings);
		} else if (button.text == exitButtonName) {
			Window::close();
		}
	}
	return result;
}

void Menu::setControlsSettings(const SettingsControls& settings) {
	auto& c = controlsUi;
	auto set = [&](i32 index, i32 value) {
		c.keycodeInputs[index].keycode = static_cast<KeyCode>(value);
	};
	set(c.leftKeycodeInputIndex, settings.left);
	set(c.rightKeycodeInputIndex, settings.right);
	set(c.jumpKeycodeInputIndex, settings.jump);
	set(c.activateKeycodeInputIndex, settings.activate);
}

SettingsControls Menu::getControlsSettings() const {
	auto& c = controlsUi;
	auto get = [&](i32 index) -> i32 {
		return static_cast<i32>(c.keycodeInputs[index].keycode);
	};
	return SettingsControls{
		.left = get(c.leftKeycodeInputIndex),
		.right = get(c.rightKeycodeInputIndex),
		.jump = get(c.jumpKeycodeInputIndex),
		.activate = get(c.activateKeycodeInputIndex),
	};
}

Menu::Event Menu::updateSoundSettingsUi(f32 dt) {
	Event result = Event::NONE;
	soundSettingsUi.layout.update(camera);
	soundSettingsUi.selection.update();

	for (auto& sliderInput : soundSettingsUi.sliderInputs) {
		const auto isSelected = soundSettingsUi.selection.isSelected(sliderInput.selectionId);
		updateHoverAnimation(
			sliderInput.hoverAnimationT,
			isSelected,
			dt);

		if (!isSelected) {
			continue;
		}
		if (Input::isKeyDownWithAutoRepeat(KeyCode::LEFT)) {
			sliderInput.value -= 0.1f;
		}
		if (Input::isKeyDownWithAutoRepeat(KeyCode::RIGHT)) {
			sliderInput.value += 0.1f;
		}
		sliderInput.value = std::clamp(sliderInput.value, 0.0f, 1.0f);
	}

	for (auto& button : soundSettingsUi.buttons) {
		const auto isSelected = soundSettingsUi.selection.isSelected(button.selectionId);
		updateHoverAnimation(
			button.hoverAnimationT,
			isSelected,
			dt);

		const auto pressed = Input::isKeyDown(KeyCode::ENTER);
		if (!isSelected || !pressed) {
			continue;
		}

		if (button.text == saveButtonName) {
			currentScreen = UiScreen::MAIN;
			result = Event::SAVE_SOUND_SETTINGS;
		} else if (button.text == cancelButtonName) {
			currentScreen = UiScreen::MAIN;
		}
	}

	drawTitle("volume", soundSettingsUi.layout.blocks[soundSettingsUi.titleId]);

	for (const auto& sliderInput : soundSettingsUi.sliderInputs) {
		const auto block = soundSettingsUi.layout.blocks[sliderInput.id];
		const auto spacingBetween = camera.aabb().size().y * 0.02f;
		const auto sizeY = block.worldSize();
		{
			const auto info = renderer.fontRenderer.getTextInfo(renderer.font, sizeY, sliderInput.name);
			Vec2 position = Vec2(camera.pos.x - info.size.x - spacingBetween, block.worldCenter());
			position.y -= info.bottomY;
			position.y -= info.size.y / 2.0f;
			const auto offset = hoverAnimationTToOffset(sliderInput.hoverAnimationT);
			drawText(sliderInput.name, position, sizeY, offset);
		}
		{
			const auto sizeY = block.worldSize();
			Vec2 min = Vec2(camera.pos.x + spacingBetween, block.worldPositionBottomY);
			Vec2 max = Vec2(camera.aabb().size().y * 0.3f, min.y + sizeY);
			Dbg::drawFilledAabb(
				Aabb::fromCorners(min, Vec2(lerp(min.x, max.x, sliderInput.value), max.y)), 
				Vec3(0.65f));
			Vec2 off(2.0f);
			//Dbg::drawAabb(Aabb::fromCorners(min - off, max - off), Vec3(0.0f, 1.0f, 1.0f), 2.0f);
			//Dbg::drawAabb(Aabb::fromCorners(min + off, max + off), Vec3(0.0f, 0.0f, 1.0f), 2.0f);
			Dbg::drawAabb(Aabb::fromCorners(min, max), Vec3(1.0f, 1.0f, 1.0f), 1.5f);
		}
	}

	for (const auto& button : soundSettingsUi.buttons) {
		drawButton(button, soundSettingsUi.layout);
	}
	return result;
}

void Menu::drawTitle(const char* text, const UiLayout::Block& block) {
	const auto height = block.worldSize();
	const auto info = renderer.fontRenderer.getTextInfo(renderer.font, height, text);
	Vec2 position = Vec2(camera.pos.x, block.worldCenter());
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	drawText(text, position, height, 0.0f);
}

void Menu::setAudioSettings(const SettingsAudio& settings) {
	auto& u = soundSettingsUi;
	auto set = [&](i32 index, f32 value) {
		u.sliderInputs[index].value = value;
	};
	set(u.masterVolumeSliderIndex, settings.masterVolume);
	set(u.musicVolumeSliderIndex, settings.musicVolume);
	set(u.soundEffectVolumeSliderIndex, settings.soundEffectVolume);
}

SettingsAudio Menu::getAudioSettings() const {
	auto& u = soundSettingsUi;
	auto get = [&](i32 index) -> f32 {
		return u.sliderInputs[index].value;
	};
	return SettingsAudio {
		.masterVolume = get(u.masterVolumeSliderIndex),
		.soundEffectVolume = get(u.soundEffectVolumeSliderIndex),
		.musicVolume = get(u.musicVolumeSliderIndex),
	};
}

Menu::Event Menu::updateControlsUi(f32 dt) {
	Event result = Event::NONE;
	controlsUi.layout.update(camera);

	bool inInputMode = false;
	for (auto& keycodeInput : controlsUi.keycodeInputs) {
		if (keycodeInput.inInputMode) {
			inInputMode = true;
		}
	}
	for (auto& keycodeInput : controlsUi.keycodeInputs) {
		updateHoverAnimation(
			keycodeInput.hoverAnimationT,
			controlsUi.selection.isSelected(keycodeInput.selectionId),
			dt);

		const auto keycode = Input::lastKeycodeDownThisFrame();
		if (keycodeInput.inInputMode && keycode.has_value()) {
			keycodeInput.inInputMode = false;
			keycodeInput.keycode = *keycode;
		} else if (!inInputMode &&
			controlsUi.selection.isSelected(keycodeInput.selectionId) && 
			Input::isKeyDown(KeyCode::ENTER)) {
			keycodeInput.inInputMode = true;
		}
	}
	if (!inInputMode) {
		controlsUi.selection.update();
	}

	for (auto& button : controlsUi.buttons) {
		const auto isSelected = controlsUi.selection.isSelected(button.selectionId);
		updateHoverAnimation(
			button.hoverAnimationT,
			isSelected,
			dt);

		const auto pressed = Input::isKeyDown(KeyCode::ENTER);
		if (!isSelected || !pressed) {
			continue;
		}

		if (button.text == saveButtonName) {
			currentScreen = UiScreen::MAIN;
			result = Event::SAVE_CONTROLS;
		} else if (button.text == cancelButtonName) {
			currentScreen = UiScreen::MAIN;
		}
	}

	drawTitle("controls", controlsUi.layout.blocks[controlsUi.titleId]);

	for (const auto& keycodeInput : controlsUi.keycodeInputs) {
		const auto block = controlsUi.layout.blocks[keycodeInput.id];
		const auto spacingBetween = camera.aabb().size().y * 0.02f;
		const auto sizeY = block.worldSize();
		{
			const auto info = renderer.fontRenderer.getTextInfo(renderer.font, sizeY, keycodeInput.name);
			Vec2 position = Vec2(camera.pos.x - info.size.x - spacingBetween, block.worldCenter());
			position.y -= info.bottomY;
			position.y -= info.size.y / 2.0f;
			const auto offset = hoverAnimationTToOffset(keycodeInput.hoverAnimationT);
			drawText(keycodeInput.name, position, sizeY, offset);
		}
		{
			const auto text = toString(keycodeInput.keycode);
			const auto sizeY = block.worldSize();
			const auto info = renderer.fontRenderer.getTextInfo(renderer.font, sizeY, text);
			Vec2 position = Vec2(camera.pos.x + spacingBetween, block.worldCenter());
			position.y -= info.bottomY;
			position.y -= info.size.y / 2.0f;
			const auto offset = keycodeInput.inInputMode ? hoverAnimationTToOffset(1.0f) : 0.0f;
			drawText(text, position, sizeY, offset);
		}
	}

	for (const auto& button : controlsUi.buttons) {
		drawButton(button, controlsUi.layout);
	}

	return result;
}

Menu::Event Menu::updateGamePausedUi(f32 dt, const Settings& settings) {
	Event result = Event::NONE;
	gamePausedUi.layout.update(camera);
	gamePausedUi.selection.update();

	drawText("game paused", gamePausedUi.layout, gamePausedUi.titleId, -0.002f);
	for (const auto& button : gamePausedUi.buttons) {
		drawText(button.text, gamePausedUi.layout, button.id, -button.hoverAnimationT * 0.0035f);
	}

	for (auto& button : gamePausedUi.buttons) {
		const auto isSelected = gamePausedUi.selection.isSelected(button.selectionId);
		updateHoverAnimation(button.hoverAnimationT, isSelected, dt);

		const auto pressed = Input::isKeyDown(KeyCode::ENTER);
		if (!isSelected || !pressed) {
			continue;
		}

		if (button.text == resumeButtonName) {
			result = Event::RESUME_GAME;
		} else if (button.text == soundSettingsButtonName) {
			changeToAudioSettingsUi(settings);
		} else if (button.text == controlsButtonName) {
			changeToControlsUi(settings);
		} else if (button.text == mainMenuButtonName) {
			result = Event::TRANSITON_TO_MAIN_MENU;
		} else if (button.text == exitButtonName) {
			Window::close();
		}
	}
	return result;
}
void Menu::drawButton(const Button& button, const UiLayout& layout) {
	auto& block = layout.blocks[button.id];
	const auto height = block.worldSize();
	const auto info = renderer.fontRenderer.getTextInfo(renderer.font, height, button.text);
	Vec2 position = Vec2(camera.pos.x, block.worldCenter());
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	drawText(button.text, position, height, hoverAnimationTToOffset(button.hoverAnimationT));
}

void UiLayout::addPadding(f32 sizeY) {
	totalSizeY += sizeY;
}

i32 UiLayout::addBlock(f32 sizeY) {
	const auto id = static_cast<i32>(blocks.size());
	blocks.push_back(Block{
		.yPosition = totalSizeY,
		.sizeY = sizeY,
	});
	totalSizeY += sizeY;
	return id;
}

void UiLayout::update(const Camera& camera) {
	const auto viewCenter = camera.pos;
	const auto viewSize = camera.aabb().size();
	const auto topY =  viewCenter.y + (totalSizeY / 2.0f) * viewSize.y;
	const auto bottomY = viewCenter.y - (totalSizeY / 2.0f) * viewSize.y;
	for (auto& block : blocks) {
		const auto topT = block.yPosition / totalSizeY;
		const auto bottomT = (block.yPosition + block.sizeY) / totalSizeY;
		block.worldPositionBottomY = lerp(topY, bottomY, bottomT);
		block.worldPositionTopY = lerp(topY, bottomY, topT);
	}
}

f32 UiLayout::Block::worldCenter() const {
	return (worldPositionTopY + worldPositionBottomY) / 2.0f;
}

f32 UiLayout::Block::worldSize() const {
	return worldPositionTopY - worldPositionBottomY;
}

i32 SelectionLayout::add() {
	const auto index = itemCount;
	itemCount++;
	return index;
}

void SelectionLayout::update() {
	if (Input::isKeyDownWithAutoRepeat(KeyCode::UP)) {
		selectedId -= 1;
	}
	if (Input::isKeyDownWithAutoRepeat(KeyCode::DOWN)) {
		selectedId += 1;
	}
	if (selectedId < 0) {
		selectedId = itemCount - 1;
	}
	if (selectedId >= itemCount) {
		selectedId = 0;
	}
}

bool SelectionLayout::isSelected(i32 selectionId) {
	return selectedId == selectionId;
}
