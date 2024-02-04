#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Vec3.hpp>
#include <vector>
#include <imgui/imgui.h>

struct PolynomialInterpolationDemo {
	PolynomialInterpolationDemo();
	void update();

	struct Polynomial {
		std::vector<Vec2T<double>> controlPoints;
		std::vector<float> coefficients;
		Vec3 color;
		bool recalculateCoefficients = false;
		Vec2 inputPoint = Vec2(0.0f);

		ImVec4 imColor() const;
	};
	int dragPointsIds = 0;

	std::vector<Polynomial> polynomials;
};