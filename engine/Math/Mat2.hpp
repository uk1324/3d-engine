#pragma once

#include <Types.hpp>
#include "Vec2.hpp"

// Row major
template<typename T>
struct Mat2T {
	Mat2T() = default;
	constexpr Mat2T(const Vec2T<T>& column0, const Vec2T<T>& column1);
	//static Mat2T fromColumns(const Vec2T<T>& column0, const Vec2T<T>& column1);
	static Mat2T fromRows(const Vec2T<T>& row0, const Vec2T<T>& row1);

	static auto rotate(T angle) -> Mat2T;

	auto transposed() const -> Mat2T;
	auto orthonormalInv() const -> Mat2T;
	auto inversed() const -> Mat2T;

	auto det() const -> T;
	auto trace() const -> T;
	//auto sqrt() const -> Mat2T;
	//auto removedScaling() const -> Mat2T;
	//
	//auto x() const -> Vec2T<T>;
	//auto y() const -> Vec2T<T>;

	Mat2T operator*(const Mat2T& other) const;
	Mat2T operator+(const Mat2T& other) const;
	Mat2T& operator+=(const Mat2T& other);
	//auto operator*(T s) const -> Mat2T;

	T& operator()(i32 x, i32 y);
	const T& operator()(i32 x, i32 y) const;
	Vec2T<T>& operator()(i32 i);
	const Vec2T<T>& operator()(i32 i) const;

	Vec2T<T> row0() const;
	Vec2T<T> row1() const;

	Vec2T<T> columns[2];
	static const Mat2T identity;
};

using Mat2 = Mat2T<float>;

template<typename T>
Vec2T<T> operator*(const Mat2T<T>& m, const Vec2T<T>& v);
template<typename T>
Vec2T<T>& operator*=(Vec2T<T>& v, const Mat2T<T>& m);

//template<typename T>
//constexpr Mat2T<T>::Mat2T(const Vec2T<T>& column0, const Vec2T<T>& column1)
//	: columns{ column0, column1 } {}

template<typename T>
constexpr Mat2T<T>::Mat2T(const Vec2T<T>& column0, const Vec2T<T>& column1) 
	: columns{ column0, column1 } {}

//template<typename T>
//Mat2T<T> Mat2T<T>::fromColumns(const Vec2T<T>& column0, const Vec2T<T>& column1) {
//	return Mat2T{ { column0, column1 } };
//}

template<typename T>
Mat2T<T> Mat2T<T>::fromRows(const Vec2T<T>& row0, const Vec2T<T>& row1) {
	return Mat2T(Vec2(row0.x, row1.x), Vec2(row0.y, row1.y));
}

template<typename T>
auto Mat2T<T>::rotate(T angle) -> Mat2T {
	const auto s = sin(angle), c = cos(angle);
	return Mat2T{ Vec2{ c, s }, Vec2{ -s, c } };
}

template<typename T>
auto Mat2T<T>::transposed() const -> Mat2T {
	const auto& m = *this;
	return Mat2T{
		Vec2T{ m(0, 0), m(1, 0) },
		Vec2T{ m(0, 1), m(1, 1) }
	};
}

template<typename T>
auto Mat2T<T>::orthonormalInv() const -> Mat2T {
	return transposed();
}

template<typename T>
auto Mat2T<T>::inversed() const -> Mat2T {
	const auto d = det();
	/*if (d == 0.0f) {
		ASSERT_NOT_REACHED();
	}*/
	const auto& m = *this;
	return Mat2T{
		Vec2T(m(1, 1) / d, -m(0, 1) / d),
		Vec2T(-m(1, 0) / d, m(0, 0) / d),
	};
}

template<typename T>
auto Mat2T<T>::det() const -> T {
	const auto& m = *this;
	return m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1);
}

template<typename T>
auto Mat2T<T>::trace() const -> T {
	const auto& m = *this;
	return m(0, 0) + m(1, 1);
}

//template<typename T>
//auto Mat2T<T>::sqrt() const -> Mat2T {
//	// https://en.wikipedia.org/wiki/Square_root_of_a_2_by_2_matrix
//	const auto delta = det();
//	const auto tau = trace();
//	const auto s = ::sqrt(delta);
//	const auto t = ::sqrt(tau);
//	return Mat2{ Vec2{ m[0][0] + s, m[0][1] }, Vec2{ m[1][0], m[1][1] + s } } * (1.0f / t);
//}
//
//template<typename T>
//auto Mat2T<T>::removedScaling() const -> Mat2T {
//	return Mat2T{
//		x().normalized(),
//		y().normalized()
//	};
//}
//
//template<typename T>
//auto Mat2T<T>::x() const -> Vec2T<T> {
//	return Vec2T{ m[0][0], m[0][1] };
//}
//
//template<typename T>
//auto Mat2T<T>::y() const -> Vec2T<T> {
//	return Vec2T{ m[1][0], m[1][1] };
//}
//
//template<typename T>
//Mat2T<T> Mat2T<T>::operator*(const Mat2T& other) const{
//	return Mat2{
//		x() * other,
//		y() * other,
//	};
//}

template<typename T>
Mat2T<T> Mat2T<T>::operator+(const Mat2T<T>& other) const{
	return Mat2T{
		columns[0] + other.columns[0],
		columns[1] + other.columns[1],
	};
}

template<typename T> 
Mat2T<T>& Mat2T<T>::operator+=(const Mat2T<T>& other) {
	*this = *this + other;
	return *this;
}
//
//template<typename T>
//auto Mat2T<T>::operator*(T s) const -> Mat2T {
//	return Mat2{
//		x() * s,
//		y() * s
//	};
//}
//
template<typename T>
T& Mat2T<T>::operator()(i32 x, i32 y) {
	// Could just reinterpret cast to array.
	return columns[x][y];
}

template<typename T>
const T& Mat2T<T>::operator()(i32 x, i32 y) const {
	return columns[x][y];
}

template<typename T>
Vec2T<T>& Mat2T<T>::operator()(i32 i) {
	return columns[i];
}
template<typename T>
const Vec2T<T>& Mat2T<T>::operator()(i32 i) const {
	return columns[i];
}

template<typename T>
Vec2T<T> Mat2T<T>::row0() const {
	const auto& m = *this;
	return Vec2T(m(0, 0), m(1, 0));
}

template<typename T>
Vec2T<T> Mat2T<T>::row1() const {
	const auto& m = *this;
	return Vec2T(m(0, 1), m(1, 1));
}

//
//template<typename T>
//auto operator*(const Vec2T<T>& v, const Mat2T<T>& m) -> Vec2T<T> {
//	return Vec2{
//		v.x * m[0][0] + v.y * m[1][0],
//		v.x * m[0][1] + v.y * m[1][1]
//	};
//}
//
//template<typename T>
//auto operator*=(Vec2T<T>& v, const Mat2T<T>& m) -> Vec2T<T>& {
//	v = v * m;
//	return v;
//}
//
//template<typename T>
//const Mat2T<T> Mat2T<T>::identity = Mat2T<T>{ Vec2T<T>{ 1, 0 }, Vec2T<T>{ 0, 1 } };

template<typename T>
Vec2T<T> operator*(const Mat2T<T>& m, const Vec2T<T>& v) {
	return v.x * m(0) + v.y * m(1);
}

template<typename T>
Vec2T<T>& operator*=(Vec2T<T>& v, const Mat2T<T>& m) {
	v = m * v;
	return *this;
}

template<typename T>
Mat2T<T> Mat2T<T>::operator*(const Mat2T& other) const {
	return Mat2T<T>(other * columns[0], other * columns[1]);
}

template<typename T>
const Mat2T<T> Mat2T<T>::identity = Mat2T<T>(Vec2T<T>(1, 0), Vec2T<T>(0, 1));