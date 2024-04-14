#pragma once

#include "Camera.hpp"

Vec2 cursorPosWorldSpace(const Camera& camera);
void zoomOnCursorPos(Camera& camera, f32 dt, f32 speed = 15.0f);