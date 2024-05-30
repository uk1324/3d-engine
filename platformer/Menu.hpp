#pragma once

#include <platformer/GameRenderer.hpp>
#include <platformer/SettingsData.hpp>
#include <engine/Input/Input.hpp>

struct UiLayout {
	f32 totalSizeY = 0.0f;
	struct Block {
		// positions are precentages of full screen size
		f32 yPosition;
		// from 0 to 1 where 1 is the full screen.
		f32 sizeY;
		// world space
		f32 worldPositionTopY = -1.0f;
		f32 worldPositionBottomY = -1.0f;

		f32 worldCenter() const;
		f32 worldSize() const;
	};
	std::vector<Block> blocks;

	void addPadding(f32 sizeY);
	i32 addBlock(f32 sizeY);

	void update(const Camera& camera);
};

struct SelectionLayout {
	i32 itemCount = 0;
	i32 add();
	i32 selectedId = 0;
	void update();
	bool isSelected(i32 selectionId);
};

struct Menu {
	// Setting this to a big number so if accessed it hopefully crashes.
	static constexpr i32 INVALID = 0xFFFFFF;

	Menu(GameRenderer& renderer);

	enum class Event {
		NONE,
		TRANSITION_TO_GAME,
		SAVE_SOUND_SETTINGS,
		SAVE_CONTROLS,
	};

	enum class UiScreen {
		MAIN,
		SOUND_SETTINGS,
		CONTROLS,
	};
	UiScreen currentScreen = UiScreen::MAIN;

	struct Text {
		std::string_view text;
		Vec2 position = Vec2(0.0f);
		f32 sizeY;
		Aabb aabb = Aabb(Vec2(0.0f), Vec2(0.0f));
		f32 hoverAnimationT = 0.0f;
	};

	struct Button {
		i32 id = INVALID;
		i32 selectionId = INVALID;
		std::string_view text;
		f32 hoverAnimationT = 0.0f;
	};

	struct MainMenuUi {
		UiLayout layout;
		SelectionLayout selection;
		i32 titleId = INVALID;
		std::vector<Button> buttons;
	};
	MainMenuUi mainMenuUi;

	struct KeycodeInput {
		std::string_view name;
		i32 id = INVALID;
		i32 selectionId = INVALID;
		KeyCode keycode;
		f32 hoverAnimationT = 0.0f;
		bool inInputMode = false;
	};

	struct ControlsUi {
		UiLayout layout;
		SelectionLayout selection;
		i32 titleId = INVALID;
		std::vector<KeycodeInput> keycodeInputs;
		std::vector<Button> buttons;

		i32 leftKeycodeInputIndex = INVALID;
		i32 rightKeycodeInputIndex = INVALID;
		i32 jumpKeycodeInputIndex = INVALID;
		i32 activateKeycodeInputIndex = INVALID;
	};
	ControlsUi controlsUi;

	void setControlsSettings(const SettingsControls& settings);
	SettingsControls getControlsSettings() const;

	struct SliderInput {
		std::string_view name;
		i32 id = INVALID;
		i32 selectionId = INVALID;
		f32 value = 0.0f; // from 0 to 1
		f32 hoverAnimationT = 0.0f;
	};

	struct SoundSettingsUi {
		UiLayout layout;
		SelectionLayout selection;
		i32 titleId = INVALID;
		std::vector<SliderInput> sliderInputs;
		std::vector<Button> buttons;

		i32 masterVolumeSliderIndex = INVALID;
		i32 soundEffectVolumeSliderIndex = INVALID;
		i32 musicVolumeSliderIndex = INVALID;
	};
	SoundSettingsUi soundSettingsUi;

	void setAudioSettings(const SettingsAudio& settings);
	SettingsAudio getAudioSettings() const;

	Event updateControlsUi(f32 dt);
	Event updateMainMenuUi(f32 dt);
	Event updateSoundSettingsUi(f32 dt);

	void drawText(std::string_view text, const UiLayout& layout, i32 id, f32 offset);

	f32 totalHeight = 0.0f;
	std::vector<Text> texts;

	Event update(f32 dt);
	void drawTextCentered(std::string_view text, Vec2 position, f32 height, f32 offset);
	void drawText(std::string_view text, Vec2 bottomLeftPosition, f32 height, f32 offset);
	Aabb getTextAabb(std::string_view text, Vec2 position, f32 height);

	void drawTitle(const char* text, const UiLayout::Block& block);
	void drawButton(const Button& button, const UiLayout& layout);

	Camera camera;
	GameRenderer& renderer;
};