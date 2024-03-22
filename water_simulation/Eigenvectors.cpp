#include "Eigenvectors.hpp"

std::array<Eigenvector, 2> computeEigenvectors(const Mat2& m) {
	float d = m.det();
	float t = (m[0][0] + m[1][1]);
	const auto s = sqrt(std::complex((t * t) / 4.0f - d, 0.0f));
	const auto l0 = t / 2.0f + s;
	const auto l1 = t / 2.0f - s;
	// https://people.math.harvard.edu/~knill/teaching/math21b2004/exhibits/2dmatrices/index.html
	// Not sure why, but the formulas had to be transposed. Maybe a problem with the input, but I don't think it is.
	if (m[1][0] != 0.0f) {
		return {
			Eigenvector{ Vec2T<Complex32>(l0 - m[1][1], m[1][0]), l0 },
			Eigenvector{ Vec2T<Complex32>(l1 - m[1][1], m[1][0]), l1 },
		};
	} else if (m[0][1] != 0.0f) {
		return {
			Eigenvector{ Vec2T<Complex32>(m[0][1], l0 - m[0][0]), l0 },
			Eigenvector{ Vec2T<Complex32>(m[0][1], l1 - m[0][0]), l1 },
		};
	} else {
		return{
			Eigenvector{ Vec2T<Complex32>(1.0f, 0.0f), l0 },
			Eigenvector{ Vec2T<Complex32>(0.0f, 1.0f), l1 },
		};
	}
}