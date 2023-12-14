#pragma once

#include "OdeRhsFunction.hpp"

template<typename StateVector>
StateVector eulerStep(const OdeRhsFunction<StateVector> auto& rhs, StateVector currentState, float currentT, float dt) {
	return currentState + dt * rhs(currentState, currentT);
}