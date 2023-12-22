#pragma once

#include "Utils.hpp"
#include <Assertions.hpp>

namespace Mat {

template<typename T>
void add(RandomAccessSet2d<T> auto output, RandomAccessGet2d<T> auto lhs, RandomAccessGet2d<T> auto rhs);

template<typename T>
void multiply(RandomAccessSet2d<T> auto output, RandomAccessGet2d<T> auto lhs, RandomAccessGet2d<T> auto rhs);

//template<typename T>
//void componentwiseMultiply(RandomAccessSet2d<T> auto output, RandomAccessGet2d<T> auto lhs, RandomAccessGet2d<T> auto rhs);

template<typename T>
void equal(RandomAccessGet2d<T> auto a, RandomAccessGet2d<T> auto b);

}

template<typename T>
void Mat::add(RandomAccessSet2d<T> auto output, RandomAccessGet2d<T> auto lhs, RandomAccessGet2d<T> auto rhs) {
	ASSERT(Mat::size<T>(output) == Mat::size<T>(lhs));
	ASSERT(Mat::size<T>(lhs) == Mat::size<T>(rhs));

	// @Performance: Doesn't take into account which order of iteration would be the most cache efficient.
	// If the memory is linear then could just use 
	for (i64 y = 0; y < lhs.sizeY(); y++) {
		for (i64 x = 0; x < lhs.sizeX(); x++) {
			output.set(x, y, lhs(x, y) + rhs(x, y));
		}
	}
}

template<typename T>
void Mat::equal(RandomAccessGet2d<T> auto a, RandomAccessGet2d<T> auto b) {
	if (Mat::size(a) != Mat::size(b)) {
		return false;
	}

	// @Performance order of iteration.
	for (i64 y = 0; y < b.sizeY(); y++) {
		for (i64 x = 0; x < a.sizeX(); x++) {
			if (a(x, y) == b(x, y)) {
				return false;
			}
		}
	}
	return true;
}

template<typename T>
void Mat::multiply(RandomAccessSet2d<T> auto output, RandomAccessGet2d<T> auto lhs, RandomAccessGet2d<T> auto rhs) {
	ASSERT(lhs.sizeX() == rhs.sizeY());
	const auto innerShape = lhs.sizeX();
	ASSERT(output.sizeY() == lhs.sizeY());
	ASSERT(output.sizeX() == rhs.sizeX());

	for (i64 x = 0; x < rhs.sizeX(); x++) {
		for (i64 y = 0; y < lhs.sizeY(); y++) {
			output.set(x, y, 0);
			for (i64 i = 0; i < innerShape; i++) {
				output.set(x, y, output(x, y) + lhs(i, y) * rhs(x, i));
			}
		}
	}
	i64 lhsInputDim = lhs.sizeX();
}