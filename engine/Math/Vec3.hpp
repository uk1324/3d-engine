#pragma once

#include <cmath>
#include <ostream>
#include <HashCombine.hpp>

template<typename T>
struct Vec3T {
	Vec3T() {};
	explicit constexpr Vec3T(const T& v);
	constexpr Vec3T(const T& x, const T& y, const T& z);
	template<typename U>
	constexpr Vec3T(const Vec3T<U>& v);

	auto applied(T(*function)(T)) const -> Vec3T;
	auto operator*(const Vec3T& v) const -> Vec3T;
	auto operator*=(const Vec3T& v) -> Vec3T&;
	auto operator*(const T& s) const -> Vec3T;
	auto operator+(const Vec3T& v) const -> Vec3T;
	auto operator+=(const Vec3T& v) -> Vec3T&;
	auto operator-(const Vec3T& v) const -> Vec3T;
	Vec3T operator-=(const Vec3T& v);
	Vec3T operator-() const;
	constexpr auto operator/(const T& s) const -> Vec3T;
	auto operator/=(const T& s) -> Vec3T&;
	auto length() const -> float;
	auto normalized() const -> Vec3T;
	Vec3T roundedToDecimalDigits(int digitsCount) const;
	T distanceTo(Vec3T p) const;
	T distanceSquaredTo(Vec3T p) const;
	Vec3T min(const Vec3T& other) const;
	Vec3T max(const Vec3T& other) const;

	bool operator==(const Vec3T&) const = default;

	auto data() -> T*;
	auto data() const -> const T*;

	// Left handed coordinate system
	static const Vec3T RIGHT;
	static const Vec3T LEFT;
	static const Vec3T DOWN;
	static const Vec3T UP;
	static const Vec3T FORWARD;
	static const Vec3T BACK;

	T x, y, z;
};

using Vec3 = Vec3T<float>;

template<typename T>
Vec3T<T> operator*(const T& s, const Vec3T<T>& v);

template<typename T>
Vec3T<T> cross(const Vec3T<T>& a, const Vec3T<T>& b);

template<typename T>
T dot(const Vec3T<T>& a, const Vec3T<T>& b);

template<typename T>
struct std::hash<Vec3T<T>> {
	std::size_t operator()(const Vec3T<T>& value) const noexcept {
		auto r = std::hash<T>()(value.x);
		r = hashCombine(r, std::hash<T>()(value.y));
		r = hashCombine(r, std::hash<T>()(value.z));
		return r;
	}
};

template<typename T>
std::ostream& operator<< (std::ostream& os, const Vec3T<T>& v) {
	os << '[' << v.x << ", " << v.y << ", " << v.z << ']';
	return os;
}

template<typename T>
constexpr Vec3T<T>::Vec3T(const T& v) 
	: x{ v }
	, y{ v } 
	, z{ v } {}

template<typename T>
constexpr Vec3T<T>::Vec3T(const T& x, const T& y, const T& z) 
	: x{ x } 
	, y{ y }
	, z{ z } {}

template<typename T>
auto Vec3T<T>::applied(T(*function)(T)) const -> Vec3T {
	return Vec3T{ function(x), function(y), function(z) };
}

template<typename T>
auto Vec3T<T>::operator*(const Vec3T& v) const -> Vec3T {
	return { x * v.x, y * v.y, z * v.z };
}

template<typename T>
auto Vec3T<T>::operator*=(const Vec3T& v) -> Vec3T& {
	*this = *this * v;
	return *this;
}

template<typename T>
auto Vec3T<T>::operator*(const T& s) const -> Vec3T {
	return Vec3T{ x * s, y * s, z * s };
}

template<typename T>
auto Vec3T<T>::operator+(const Vec3T& v) const -> Vec3T {
	return Vec3T{ x + v.x, y + v.y, z + v.z };
}

template<typename T>
auto Vec3T<T>::operator+=(const Vec3T& v) -> Vec3T& {
	*this = *this + v;
	return *this;
}

template<typename T>
auto Vec3T<T>::operator-(const Vec3T& v) const -> Vec3T {
	return Vec3T{ x - v.x, y - v.y, z - v.z };
}

template<typename T>
Vec3T<T> Vec3T<T>::operator-=(const Vec3T& v) {
	*this = *this - v;
	return *this;
}

template<typename T>
Vec3T<T> Vec3T<T>::operator-() const {
	return Vec3T(-x, -y, -z);
}

template<typename T>
constexpr auto Vec3T<T>::operator/(const T& s) const -> Vec3T {
	return { x / s, y / s, z / s };
}

template<typename T>
auto Vec3T<T>::operator/=(const T& s) -> Vec3T& {
	*this = *this / s;
	return *this;
}

template<typename T>
auto Vec3T<T>::length() const -> float {
	return sqrt(x * x + y * y + z * z);
}

template<typename T>
auto Vec3T<T>::normalized() const -> Vec3T {
	const auto l = length();
	if (l == 0.0f) {
		return *this;
	}
	return *this / l;
}

template<typename T>
Vec3T<T> Vec3T<T>::roundedToDecimalDigits(int digitsCount) const {
	const auto scale = pow(10.0f, digitsCount);
	return (*this * scale).applied(floor) / scale;
}

template<typename T>
T Vec3T<T>::distanceTo(Vec3T p) const {
	return sqrt(distanceSquaredTo(p));
}

template<typename T>
T Vec3T<T>::distanceSquaredTo(Vec3T p) const {
	return (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y) + (z - p.z) * (z - p.z);
}

template<typename T>
Vec3T<T> Vec3T<T>::min(const Vec3T& other) const {
	return Vec3(std::min(x, other.x), std::min(y, other.y), std::min(z, other.z));
}

template<typename T>
Vec3T<T> Vec3T<T>::max(const Vec3T& other) const {
	return Vec3(std::max(x, other.x), std::max(y, other.y), std::max(z, other.z));
}

template<typename T>
inline auto Vec3T<T>::data() -> T* {
	return &x;
}

template<typename T>
inline auto Vec3T<T>::data() const -> const T* {
	return &x;
}

template<typename T>
template<typename U>
constexpr Vec3T<T>::Vec3T(const Vec3T<U>& v)
	: x{ static_cast<T>(v.x) }
	, y{ static_cast<T>(v.y) }
	, z{ static_cast<T>(v.z) } {}

template<typename T>
const Vec3T<T> Vec3T<T>::RIGHT(1.0f, 0.0f, 0.0f);
template<typename T>
const Vec3T<T> Vec3T<T>::LEFT(-1.0f, 0.0f, 0.0f);
template<typename T>
const Vec3T<T> Vec3T<T>::DOWN(0.0f, -1.0f, 0.0f);
template<typename T>
const Vec3T<T> Vec3T<T>::UP(0.0f, 1.0f, 0.0f);
template<typename T>
const Vec3T<T> Vec3T<T>::FORWARD(0.0f, 0.0f, 1.0f);
template<typename T>
const Vec3T<T> Vec3T<T>::BACK(0.0f, 0.0f, -1.0f);

template<typename T>
Vec3T<T> cross(const Vec3T<T>& a, const Vec3T<T>& b) {
	return Vec3T<T>(
		(a.y * b.z) - (a.z * b.y),
		(a.z * b.x) - (a.x * b.z),
		(a.x * b.y) - (a.y * b.x)
	);
}

template<typename T>
T dot(const Vec3T<T>& a, const Vec3T<T>& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
Vec3T<T> operator*(const T& s, const Vec3T<T>& v) {
	return v * s;
}