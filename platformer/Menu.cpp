#include "Menu.hpp"
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <engine/Math/Utils.hpp>
#include <engine/Window.hpp>
#include <framework/Dbg.hpp>

Menu::Menu(GameRenderer& renderer)
	: renderer(renderer) {
	camera.zoom /= 280.0f;
}

void Menu::update(GameRenderer& renderer) {
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, Window::size().x, Window::size().y);
	camera.aspectRatio = Window::aspectRatio();

	f32 logoSize = 0.15f;
	struct Text {
		const char* text;
		Vec2 position = Vec2(0.0f);
		Vec2 size = Vec2(0.0f);
	};
	std::vector<Text> texts;
	f32 totalHeight = 0.0f;
	auto addText = [&](const char* text, f32 height) {
		texts.push_back(Text{
			.text = text,
			.position = Vec2(0.0f, totalHeight),
			.size = Vec2(0.0f, height)
		});
		totalHeight += height;
	};
	auto addPadding = [&](f32 padding) {
		totalHeight += padding;
	};
	addPadding(0.2f);
	f32 buttonSize = 0.05f;
	addText("AXOMETRIC", 0.15f);
	addPadding(0.03f);
	addText({ "play" }, buttonSize);
	addPadding(0.03f);
	addText({ "sound settings" }, buttonSize);
	addPadding(0.03f);
	addText({ "controls" }, buttonSize);
	addPadding(0.03f);
	addText({ "exit" }, buttonSize);
	addPadding(0.2f);

	const auto view = camera.aabb();
	const auto viewSize = view.size();
	for (const auto& text : texts) {
		f32 yt = text.position.y / totalHeight;
		f32 y = lerp(view.max.y, view.min.y, yt);
		Vec2 position = Vec2(camera.pos.x, y);
		f32 sizeY = text.size.y / totalHeight * viewSize.y;
		drawTextCentered(renderer, text.text, position, sizeY);
		const auto aabb = getTextAabb(text.text, position, sizeY);
		Dbg::drawAabb(aabb, Vec3(1.0f), 4.0f);
	}
	//drawTextCentered(renderer, "test", camera.pos, 55.2f);

	//for (auto& button : buttons) {
	//	y += padding;
	//	//drawTextCentered(renderer, button.text, Vec2(camera.pos.x, lerp(view.max.x, view.min.x, y)), view.size().y * buttonSize);
	//	y += buttonSize;
	//}

	/*drawTextCentered(renderer, "AXOMETRIC", Vec2(camera.pos.x, lerp(view.min.x, view.max.x, y)), y - logoSize / 2.0f);*/
	//drawTextCentered(renderer, "AXOMETRIC", Vec2(camera.pos.x, lerp(view.max.x, view.min.x, y)), view.size().y * logoSize);
	//y += logoSize / 2.0f;

	////f32 padding = 0.02f;
	//for (auto& button : buttons) {
	//	y += padding;
	//	//drawTextCentered(renderer, button.text, Vec2(camera.pos.x, lerp(view.max.x, view.min.x, y)), view.size().y * buttonSize);
	//	y += buttonSize;
	//}

	renderer.renderBackground();
	renderer.update(); // TODO: maybe make renderer.updateTime(); or maybe store local time and pass it to the function

	renderer.renderer.camera = camera;

	glEnable(GL_BLEND);

	//drawTextCentered(renderer, "AXOMETRIC", camera.pos, 80.0f);
	
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
