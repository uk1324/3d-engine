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

enum class LinearSystem2dType {
	NON_ISOLATED_FIXED_POINTS,
	SADDLE_POINT,
	UNSTABLE_NODE,
	UNSTABLE_SPIRAL,
	UNSTABLE_DEGENERATE_NODE,
	STABLE_NODE,
	STABLE_SPIRAL,
	STABLE_DEGENERATE_NODE,
	CENTER,
};

LinearSystem2dType linearSystemType(const Mat2& m);
const char* linearSystemTypeToString(LinearSystem2dType type);