#pragma once

#include <imgui/imgui.h>
#include <Types.hpp>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Vec4.hpp>
#include <engine/Utils/Put.hpp>
#include <engine/Utils/StringStream.hpp>

namespace Gui {

void update();

void inputI32(const char* name, i32& value);
void inputFloat(const char* name, float& value);
void sliderFloat(const char* name, float& value, float min, float max);
void inputVec2(const char* name, Vec2& value);
void inputVec3(const char* name, Vec3& value);
void checkbox(const char* name, bool& value);
void inputColor(const char* name, Vec4& value);
void inputColor(const char* name, Vec3& value);

bool beginPropertyEditor();
void endPropertyEditor();
void popPropertyEditor();

bool node(const char* name);
void nodeEnd();

template<typename ...Args>
void put(const char* format, const Args&... args);

template<typename ...Args>
void put(const char* format, const Args&... args) {
	static thread_local StringStream stream;
	stream.string().clear();
	put(stream, format, args...);
	ImGui::Text("%s", stream.string().c_str());
}

}

#define GUI_PROPERTY_EDITOR(editorCode) \
	do { \
		if (Gui::beginPropertyEditor()) { \
			{ editorCode; } \
			Gui::endPropertyEditor(); \
		} \
		Gui::popPropertyEditor(); \
	} while (false)