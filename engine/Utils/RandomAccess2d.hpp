#pragma once

#include <concepts>
#include <Types.hpp>

// COMMENTS FROM SPAN2D
// TODO: This doesn't support custom strides.
// Not sure if it is better to extend this class with a optional template flag or to create a new class.
// The most generic thing I could do would be to create a function from 2 indices to some value
// So the best thing would be to create an interface a concept.


// Not using operator(), because some things may store without being able to return references. For example a bitset.
// Could get the item type from the Matrix type for example by requring the Matrix to have a using field of name ItemType
// struct Matrix { using ItemType = ? }, but this wouldn't allow constraining the ItemType.
template<typename Matrix, typename ItemType>
concept RandomAccessGet2d = requires(Matrix matrix, ItemType item, i64 x, i64 y) {
	{ matrix.get(x, y) } -> std::convertible_to<ItemType>;
};

template<typename Matrix, typename ItemType>
concept RandomAccessSet2d = RandomAccessGet2d<Matrix, ItemType> && requires(Matrix matrix, ItemType item, i64 x, i64 y) {
	matrix.set(x, y, item);
};

/*
Example usage

float abc(RandomAccessGet2d<float> auto test) {
	float x = test.get(1, 1);
	return x;
}

float data[4]{ 1, 2, 3, 4 };
Span2d<float> a(data, 2, 2);
auto x = abc(a);

*/