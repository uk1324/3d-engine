#pragma once

#include "Vec3.hpp"

template<typename T>
struct Mat3T {
	constexpr Mat3T(const Vec3T<T>& x, const Vec3T<T>& y, const Vec3T<T>& z);

	static Mat3T rotationX(const T& angle);
	static Mat3T scale(const Vec3T<T>& s);
	static Mat3T scale(const T& s);
	static Mat3T rotationY(const T& angle);

	Mat3T operator*(const Mat3T& m) const;

	const Vec3T<T>& operator[](i32 i) const;
	Vec3T<T>& operator[](i32 i);

	Vec3T<T> basis[3];
};

using Mat3 = Mat3T<float>;

template<typename T>
Vec3T<T> operator*(const Vec3T<T>& v, const Mat3T<T>& m);
template<typename T>
Vec3T<T>& operator*=(Vec3T<T>& v, const Mat3T<T>& m);

template<typename T>
constexpr Mat3T<T>::Mat3T(const Vec3T<T>& x, const Vec3T<T>& y, const Vec3T<T>& z)
	: basis{ x, y, z } {}

template<typename T>
Mat3T<T> Mat3T<T>::rotationX(const T& angle) {
	const T c = cos(angle);
	const T s = sin(angle);
	return {
		{ 1, 0, 0 },
		{ 0, c, -s },
		{ 0, s, c },
	};
}

template<typename T>
Mat3T<T> Mat3T<T>::scale(const Vec3T<T>& s) {
	return {
		{ s.x, 0, 0 },
		{ 0, s.y, 0 },
		{ 0, 0, s.z },
	};
}

template<typename T>
Mat3T<T> Mat3T<T>::scale(const T& s) {
	return {
		{ s, 0, 0 },
		{ 0, s, 0 },
		{ 0, 0, s },
	};
}

template<typename T>
Mat3T<T> Mat3T<T>::rotationY(const T& angle) {
	const T c = cos(angle);
	const T s = sin(angle);
	return {
		{ c, 0, s },
		{ 0, 1, 0 },
		{ -s, 0, c },
	};
}

template<typename T>
Mat3T<T> Mat3T<T>::operator*(const Mat3T& m) const {
	return { basis[0] * m, basis[1] * m, basis[2] * m };
}

template<typename T>
const Vec3T<T>& Mat3T<T>::operator[](i32 i) const {
	return const_cast<Mat3T<T>*>(this)->operator[](i);
}

template<typename T>
Vec3T<T>& Mat3T<T>::operator[](i32 i) {
	CHECK(i >= 0 && i <= 2);
	return basis[i];
}

template<typename T>
Vec3T<T> operator*(const Vec3T<T>& v, const Mat3T<T>& m) {
	return v.x * m[0] + v.y * m[1] + v.z * m[2];
}

template<typename T>
Vec3T<T>& operator*=(Vec3T<T>& v, const Mat3T<T>& m) {
	*this = *this * m;
	return *this;
}
