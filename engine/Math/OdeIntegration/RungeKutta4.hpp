#pragma once

#include "OdeRhsFunction.hpp"

template<typename StateVector>
StateVector rungeKutta4Step(const OdeRhsFunction<StateVector> auto& rhs, StateVector currentState, float currentT, float dt) {
	const auto halfDt = dt / 2.0f;
	const auto k1 = rhs(currentState, currentT);
	const auto k2 = rhs(currentState + halfDt * k1, currentT + halfDt);
	const auto k3 = rhs(currentState + halfDt * k2, currentT + halfDt);
	const auto k4 = rhs(currentState + dt * k3, currentT + dt);
	return currentState + (dt / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
}