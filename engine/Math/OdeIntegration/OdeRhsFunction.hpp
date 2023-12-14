#pragma once

template<typename Function, typename StateVector>
concept OdeRhsFunction = requires(Function rhs, StateVector stateVector, float t, float scalar) {
	{ rhs(stateVector, t) } -> std::convertible_to<StateVector>;
	{ scalar* stateVector } -> std::convertible_to<StateVector>;
	{ stateVector + stateVector } -> std::convertible_to<StateVector>;
};
