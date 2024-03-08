#include "Gui.hpp"
#include <string>

using namespace Gui;
using namespace ImGui;

thread_local std::string string;
static const char* prependWithHashHash(const char* str) {
	const auto offset = string.size();
	string += "##";
	string += str;
	string += '\0';
	return string.data() + offset;
}

static void leafNodeBegin(const char* name) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	ImGui::TreeNodeEx(name, flags);
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
}

void Gui::update() {
	string.clear();
}

// Remember to always use prependWithHashHash
bool Gui::inputI32(const char* name, i32& value) {
	leafNodeBegin(name);
	return ImGui::InputScalar(prependWithHashHash(name), ImGuiDataType_S32, reinterpret_cast<void*>(&value));
}

bool Gui::sliderI32(const char* name, i32& value, i32 min, i32 max) {
	leafNodeBegin(name);
	return ImGui::SliderInt(prependWithHashHash(name), &value, min, max);
}

bool Gui::inputFloat(const char* name, float& value) {
	leafNodeBegin(name);
	return ImGui::InputFloat(prependWithHashHash(name), &value);
}

bool Gui::sliderFloat(const char* name, float& value, float min, float max) {
	leafNodeBegin(name);
	return ImGui::SliderFloat(prependWithHashHash(name), &value, min, max);
}

bool Gui::inputVec2(const char* name, Vec2& value) {
	leafNodeBegin(name);
	return InputFloat2(prependWithHashHash(name), value.data());
}

bool Gui::inputVec3(const char* name, Vec3& value) {
	leafNodeBegin(name);
	return InputFloat3(prependWithHashHash(name), value.data());
}

bool Gui::checkbox(const char* name, bool& value) {
	leafNodeBegin(name);
	return ImGui::Checkbox(prependWithHashHash(name), &value);
}

bool Gui::inputColor(const char* name, Vec4& value) {
	leafNodeBegin(name);
	return ImGui::ColorEdit4(prependWithHashHash(name), value.data());
}

bool Gui::inputColor(const char* name, Vec3& value) {
	leafNodeBegin(name);
	return ImGui::ColorEdit3(prependWithHashHash(name), value.data());
}

bool Gui::inputText(const char* name, char* buffer, usize size) {
	leafNodeBegin(name);
	return ImGui::InputText(prependWithHashHash(name), buffer, size);
}

bool Gui::beginPropertyEditor(PropertyEditorFlags flags) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGuiTableFlags tableFlags = 0;
	switch (flags) {
		using enum Gui::PropertyEditorFlags;

	case TableAdjustable:
		tableFlags = ImGuiTableFlags_Resizable;
		break;
	case TableStetchToFit:
		tableFlags = ImGuiTableFlags_SizingStretchProp;
		break;
	}
	return ImGui::BeginTable("split", 2, tableFlags);
	//return ImGui::BeginTable("split", 2, ImGuiTableFlags_SizingFixedSame);
	//return ImGui::BeginTable("split", 2, ImGuiTableFlags_Resizable);
	//return ImGui::BeginTable("split", 2, ImGuiTableFlags_SizingFixedFit);
	//return ImGui::BeginTable("split", 2, ImGuiTableFlags_SizingStretchProp);
}

void Gui::endPropertyEditor() {
	ImGui::EndTable();
}

void Gui::popPropertyEditor() {
	ImGui::PopStyleVar();
}

bool Gui::node(const char* name) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	bool nodeOpen = ImGui::TreeNode(name);
	ImGui::TableSetColumnIndex(1);
	return nodeOpen;
}

void Gui::nodeEnd() {
	ImGui::TreePop();
}