#pragma once

#include "Vec3.hpp"

template<typename T>
struct QuatT {
	constexpr QuatT(const T& x, const T& y, const T& z, const T& w);
	QuatT(const T& angle, const Vec3T<T>& axis);

	QuatT<T> operator* (const QuatT<T>& rhs) const;
	Vec3T<T> operator* (const Vec3T<T>& rhs) const;

	QuatT conjugate() const;

	T x, y, z, w;
};

using Quat = QuatT<float>;

template<typename T>
constexpr QuatT<T>::QuatT(const T& x, const T& y, const T& z, const T& w) 
	: x(x)
	, y(y)
	, z(z)
	, w(w) {}

template<typename T>
QuatT<T>::QuatT(const T& angle, const Vec3T<T>& axis) {
	// TODO: Derive.
	T c = cos(angle / 2);
	T s = sin(angle / 2);
	w = c;
	x = axis.x * s;
	y = axis.y * s;
	z = axis.z * s;
}

template<typename T>
QuatT<T> QuatT<T>::operator*(const QuatT<T>& rhs) const {
	return QuatT(y * rhs.z - z * rhs.y + x * rhs.w + w * rhs.x,
		z * rhs.x - x * rhs.z + y * rhs.w + w * rhs.y,
		x * rhs.y - y * rhs.x + z * rhs.w + w * rhs.z,
		w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
}

template<typename T>
Vec3T<T> QuatT<T>::operator*(const Vec3T<T>& rhs) const {
	// https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
	QuatT<T> q = (*this * QuatT<T>(rhs.x, rhs.y, rhs.z, 0)) * conjugate();
	return Vec3T<T>(q.x, q.y, q.z);
}

template<typename T>
QuatT<T> QuatT<T>::conjugate() const {
	return QuatT<T>(-x, -y, -z, w);
}
