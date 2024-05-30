#pragma once

#include <platformer/GameRenderer.hpp>
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
	Menu(GameRenderer& renderer);

	struct Text {
		std::string_view text;
		Vec2 position = Vec2(0.0f);
		f32 sizeY;
		Aabb aabb = Aabb(Vec2(0.0f), Vec2(0.0f));
		f32 hoverAnimationT = 0.0f;
	};

	struct Button {
		i32 id = -1;
		i32 selectionId = -1;
		std::string_view text;
		f32 hoverAnimationT = 0.0f;
	};

	struct MainMenuUi {
		UiLayout layout;
		SelectionLayout selection;
		i32 titleId = -1;
		std::vector<Button> buttons;
		//i32 selectedButtonIndex = 0;
	};
	MainMenuUi mainMenuUi;

	struct KeycodeInput {
		std::string_view name;
		i32 id = -1;
		i32 selectionId = -1;
		KeyCode keycode;
		f32 hoverAnimationT = 0.0f;
		bool inInputMode = false;
	};

	struct ControlsUi {
		UiLayout layout;
		SelectionLayout selection;
		i32 titleId = -1;
		std::vector<KeycodeInput> keycodeInputs;
		std::vector<Button> buttons;
	};
	ControlsUi controlsUi;

	struct SliderInput {
		std::string_view name;
		i32 id = -1;
		i32 selectionId = -1;
		f32 value = 0.0f; // from 0 to 1
		f32 hoverAnimationT = 0.0f;
	};

	struct SoundSettingsUi {
		UiLayout layout;
		SelectionLayout selection;
		i32 titleId = -1;
		std::vector<SliderInput> sliderInputs;
		std::vector<Button> buttons;
	};
	SoundSettingsUi soundSettingsUi;

	void updateControlsUi(f32 dt);
	void updateMainMenuUi(f32 dt);
	void updateSoundSettingsUi(f32 dt);

	void drawText(std::string_view text, const UiLayout& layout, i32 id, f32 offset);

	f32 totalHeight = 0.0f;
	std::vector<Text> texts;

	void update(f32 dt);
	void drawTextCentered(std::string_view text, Vec2 position, f32 height, f32 offset);
	void drawText(std::string_view text, Vec2 bottomLeftPosition, f32 height, f32 offset);
	Aabb getTextAabb(std::string_view text, Vec2 position, f32 height);

	void drawTitle(const char* text, const UiLayout::Block& block);
	void drawButton(const Button& button, const UiLayout& layout);

	Camera camera;
	GameRenderer& renderer;
};