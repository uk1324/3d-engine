#pragma once

#include <expected>

namespace RootFinding {

template<typename Number>
struct NewtonOutput {
	Number approximateRoot;
};

enum class NewtonError {
	DERIVATIVE_EQUAL_TO_ZERO,
	MAX_ITERATION_COUNT_EXCEEDED
};

template<typename Number, typename Function>
std::expected<NewtonOutput<Number>, NewtonError> newton(Number initialGuess, Function function, Function derivative, i64 maxIterations, Number tolerance) {

	Number current = initialGuess;

	for (i64 i = 0; i < maxIterations; i++) {
		const auto currentValue = function(current);
		if (abs(currentValue) < tolerance) {
			return NewtonOutput{ current };
		}

		const auto currentDerivative = derivative(current);
		if (currentDerivative == 0.0f) {
			return std::unexpected(NewtonError::DERIVATIVE_EQUAL_TO_ZERO);
		}

		current -= currentValue / currentDerivative;
	}

	return std::unexpected(NewtonError::MAX_ITERATION_COUNT_EXCEEDED);
}

}

