#pragma once

#include "OdeRhsFunction.hpp"

template<typename StateVector>
StateVector midpointStep(const OdeRhsFunction<StateVector> auto& rhs, StateVector currentState, float currentT, float dt) {
	const auto halfDt = dt / 2.0f;
	return currentState + dt * rhs(currentState + halfDt * rhs(currentState, currentT), currentT + halfDt);
}