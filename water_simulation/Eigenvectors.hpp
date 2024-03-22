#pragma once

#include <complex>
#include <array>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Mat2.hpp>

using Complex32 = std::complex<float>;

struct Eigenvector {
	Vec2T<Complex32> eigenvector;
	Complex32 eigenvalue;
};

std::array<Eigenvector, 2> computeEigenvectors(const Mat2& m);