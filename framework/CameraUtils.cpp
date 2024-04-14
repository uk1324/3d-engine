#include "CameraUtils.hpp"
#include <engine/Input/Input.hpp>

Vec2 cursorPosWorldSpace(const Camera& camera) {
	return Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
}

void zoomOnCursorPos(Camera& camera, f32 dt, f32 speed) {
	if (const auto scroll = Input::scrollDelta(); scroll != 0.0f) {
		const auto cursorPosBeforeScroll = cursorPosWorldSpace(camera);
		const auto scrollSpeed = speed * abs(scroll);
		const auto scrollIncrement = pow(scrollSpeed, dt);
		if (scroll > 0.0f) camera.zoom *= scrollIncrement;
		else camera.zoom /= scrollIncrement;

		const auto newCursorPos = cursorPosWorldSpace(camera);

		camera.pos -= (newCursorPos - cursorPosBeforeScroll);
	}
}
