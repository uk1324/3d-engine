#pragma once

#include <expected>
#include <cmath>
#include "../Utils.hpp"

enum class BisectionResult {
	SUCCESS,
	MAX_ITERATION_COUNT_EXCEEDED, // without reaching specified tolerance
	INVALID_INPUT_INTERVAL,
	INVALID_TOLERANCE
};

template<typename Number>
struct BisectionOutput {
	BisectionResult result;
	Number input;
	Number output;
};

// Probably shouldn't use signbit because it would return the sign of +0 and -0. TODO: Find what would be the best option. a * b < 0?
//template<typename Number>
//bool oppositeSignsZeroNotcounted(Number a, Number b) {
//	return a * b > 0;
//}

// TODO: Using bisection values that are closer to zero might not be used, because there are still more loops. Could store the closest value and return that.

// TODO: Could output a vector if intervals if the option is specified. Could have a plotting function that plots intervals.

// Assumes that function is has the intermediate value property.
template<typename Number, typename Function>
BisectionOutput<Number> bisect(Number left, Number right, Function function, i64 maxIterationCount, Number functionInputTolerance) {
	BisectionOutput<Number> output;

	if (functionInputTolerance < 0.0f) {
		// 0 tolerance is valid it means continue to max iterations unless you find exacly zero.
		output.result = BisectionResult::INVALID_TOLERANCE;
		return output;
	}

	// NaN handling?
	if (right < left) {
		output.result = BisectionResult::INVALID_INPUT_INTERVAL;
		return output;
	}

	auto leftValue = function(left);
	auto rightValue = function(right);

	if (signOrZero(leftValue) * signOrZero(rightValue) > 0) {
		output.result = BisectionResult::INVALID_TOLERANCE;
		return output;
	}

	Number mid, midValue;
	
	for (i64 i = 0; i < maxIterationCount; i++) {
		// Is there a numerical difference between (a + b) / 2 and a + (b - a) / 2? Shoudln't the below be better, because there is less calculations done. I guess the below could overflow more easily.
		// a + (b - a) / 2 is better, because there might be round off errors that for example cause (left + right) / 2 to lie outside of the range [left, right]
		mid = (left + right) / 2;
		midValue = function(mid);

		if (midValue == 0 || (right - left < functionInputTolerance)) {
			output.input = mid;
			output.output = midValue;
			output.result = BisectionResult::SUCCESS;
			return output;
		} else if (signOrZero(leftValue) * signOrZero(midValue) > 0) {
			leftValue = midValue;
			left = mid;
		} else {
			rightValue = midValue;
			right = mid;
		}

		leftValue = function(left);
		rightValue = function(right);
	}

	output.input = mid;
	output.output = midValue;
	output.result = BisectionResult::MAX_ITERATION_COUNT_EXCEEDED;
	return output;
}