#include "Menu.hpp"
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <engine/Math/Utils.hpp>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <framework/Dbg.hpp>

// Making a layout by calculating the total height and scaling things so that they fit would make it so that elements (like text) change size in different screens. 
// If things don't fit on a single screen then some sort of scrolling needs to be implemented. The simplest thing is to probably just specify sizes in fractions of full screen size and just don't put too many things.
// The layout builder could return an index into a position array. This index would be stored in the retained structs that would store things like the animationT.

Menu::Menu(GameRenderer& renderer)
	: renderer(renderer) {
	camera.zoom /= 280.0f;

	f32 logoSize = 0.15f;
	auto addText = [&](const char* text, f32 height) {
		texts.push_back(Text{
			.text = text,
			.position = Vec2(0.0f, totalHeight),
			.sizeY = height,
		});
		totalHeight += height;
	};
	auto addPadding = [&](f32 padding) {
		totalHeight += padding;
	};
	addPadding(0.2f);
	f32 buttonSize = 0.05f;
	addText("AXOMETRIC", logoSize);
	addPadding(0.03f);
	addText({ "play" }, buttonSize);
	addPadding(0.03f);
	addText({ "sound settings" }, buttonSize);
	addPadding(0.03f);
	addText({ "controls" }, buttonSize);
	addPadding(0.03f);
	addText({ "exit" }, buttonSize);
	addPadding(0.2f);
}

void Menu::update(GameRenderer& renderer) {
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, Window::size().x, Window::size().y);
	camera.aspectRatio = Window::aspectRatio();

	const auto view = camera.aabb();
	const auto viewSize = view.size();
	for (auto& text : texts) {
		f32 yt = text.position.y / totalHeight;
		f32 y = lerp(view.max.y, view.min.y, yt);
		Vec2 position = Vec2(camera.pos.x, y);
		f32 sizeY = text.sizeY / totalHeight * viewSize.y;
		const auto aabb = getTextAabb(text.text, position, sizeY);
		text.aabb = aabb;
		drawTextCentered(renderer, text.text, position, sizeY);
	}

	const auto cursorPos = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
	for (auto& text : texts) {
		if (text.aabb.contains(cursorPos)) {
			text.hoverAnimationT -= 0.2f;
		} else {
			text.hoverAnimationT += 0.2f;
		}
		text.hoverAnimationT = std::clamp(text.hoverAnimationT, 0.0f, 1.0f);
	}

	for (auto& text : texts) {
		if (!text.aabb.contains(cursorPos) || !Input::isMouseButtonDown(MouseButton::LEFT)) {
			continue;
		}

		if (text.text == "exit") {
			Window::close();
		}
	}

	renderer.renderBackground();
	renderer.update(); // TODO: maybe make renderer.updateTime(); or maybe store local time and pass it to the function

	renderer.renderer.camera = camera;

	glEnable(GL_BLEND);
	renderer.fontRenderer.render(renderer.font, renderer.renderer.instancesVbo);
	glDisable(GL_BLEND);
}

void Menu::drawTextCentered(GameRenderer& renderer, std::string_view text, Vec2 position, f32 height) {
	const auto info = renderer.fontRenderer.getTextInfo(renderer.font, height, text);
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	position.y -= info.size.y / 2.0f;
	renderer.fontRenderer.addTextToDraw(
		renderer.font,
		position,
		camera.worldToCameraToNdc(),
		height,
		text
	);
}

Aabb Menu::getTextAabb(std::string_view text, Vec2 position, f32 height) {
	const auto info = renderer.fontRenderer.getTextInfo(renderer.font, height, text);
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	position.y -= info.size.y / 2.0f;
	return Aabb::fromCorners(position, position + info.size);
}

void UiLayout::addPadding(f32 sizeY) {
	totalSizeY += sizeY;
}

i32 UiLayout::addBlock(f32 sizeY) {
	const auto id = static_cast<i32>(blocks.size());
	blocks.push_back(Block{
		.yPosition = totalSizeY,
		.sizeY = sizeY,
		.aabb = Aabb(Vec2(0.0f), Vec2(0.0f))
	});
	totalSizeY += sizeY;
	return id;
}
