#pragma once

#include <RandomAccess2d.hpp>
#include "../Vec2.hpp"

namespace Mat {

template<typename T>
Vec2T<i64> size(RandomAccessGet2d<T> auto m);

template<typename T>
Vec2T<i64> size(RandomAccessGet2d<T> auto m) {
	return Vec2T<i64>(m.sizeX(), m.sizeY());
}

};
