#pragma once

#include <engine/Math/Mat2.hpp>

struct MatrixDemo {
	MatrixDemo();

	void update();

	Mat2 matrix = Mat2::identity;
	bool lock01 = false;
};