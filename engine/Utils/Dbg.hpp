#pragma once

#include <StringStream.hpp>
#include <Gui.hpp>

// For not displaying the same thing multiple times could create a static bool that stores if it was already displayed this frame. Then this bool would be registed to some object so it is reset at the end of the frame.

#define chk(name, initialValue) static bool name = (initialValue); ImGui::Checkbox(#name, &name); if (name)
#define chkbox(name) static bool name = false; ImGui::Checkbox(#name, &name);
#define infloat(name, initialValue) static float name = initialValue; ImGui::InputFloat(#name, &name)
#define insliderfloat(name, initialValue, min, max) static float name = initialValue; ImGui::SliderFloat(#name, &name, min, max)
#define inint(name, initialValue) static int name = initialValue; ImGui::InputInt(#name, &name)

// https://codereview.stackexchange.com/questions/134627/c-identity-function
template<typename T>
decltype(auto) dbgImplementation(T&& value, const char* expressionString) {
	thread_local StringStream stream;
	std::cout << expressionString << " = " << value << '\n';
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