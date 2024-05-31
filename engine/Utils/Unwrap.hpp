#pragma once

#include <engine/Log.hpp>
#include <expected>
#include <sstream>

template<typename T, typename E>
T unwrap(std::expected<T, E>&& v) {
	if (!v.has_value()) {
		std::stringstream s;
		s << v.error();
		Log::fatal("%", s.str());
	}

	auto e = std::move(*v);
	return e;
}