#pragma once

#include <StringStream.hpp>
#include <Gui.hpp>

#define chk(name) static bool name = false; ImGui::Checkbox(#name, &name); if (name)
#define chkbox(name) static bool name = false; ImGui::Checkbox(#name, &name);
#define floatin(name, value) static float name = value; ImGui::InputFloat(#name, &name)
#define intin(name, value) static int name = value; ImGui::InputInt(#name, &name)

// https://codereview.stackexchange.com/questions/134627/c-identity-function
template<typename T>
decltype(auto) dbgImplementation(T&& value, const char* expressionString) {
	thread_local StringStream stream;
	std::cout << expressionString << " = " << value;
	return std::forward<T>(value);
}

template<typename T>
decltype(auto) dbgGuiImplementation(T&& value, const char* expressionString) {
	thread_local StringStream stream;
	stream.string().clear();
	stream << value;
	ImGui::Text("%s = %s", expressionString, stream.string().c_str());
	return std::forward<T>(value);
}

#define dbg(value) dbgImplementation(value, #value)
#define dbgGui(value) dbgGuiImplementation(value, #value)