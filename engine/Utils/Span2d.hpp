#pragma once

#include <engine/Math/Vec2.hpp>

template<typename T>
class Span2d {
public:
	Span2d(T* data, i64 sizeX, i64 sizeY);

	Span2d<const T> asConst();

	T& operator()(i64 x, i64 y);
	const T& operator()(i64 x, i64 y) const;

	const T& get(i64 x, i64 y) const;
	T& get(i64 x, i64 y);
	void set(i64 x, i64 y, const T& item);

	i64 sizeX() const;
	i64 sizeY() const;
	Vec2T<i64> size() const;
	T* data() const;

private:
	T* data_;
	i64 sizeX_;
	i64 sizeY_;
};
// TODO: Make algorithms for drawing things into a span2d. One disadvantage is that these algorithms will only work for contigously stored data. But the algorithms can easily be extended to use arbitrary data access.

template<typename T>
Span2d<T>::Span2d(T* data, i64 sizeX, i64 sizeY) 
	: data_(data)
	, sizeX_(sizeX)
	, sizeY_(sizeY) {
}

template<typename T>
Span2d<const T> Span2d<T>::asConst() {
	return Span2d<const T>(data_, sizeX_, sizeY_);
}

template<typename T>
T& Span2d<T>::operator()(i64 x, i64 y) {
	return get(x, y);
}

template<typename T>
const T& Span2d<T>::operator()(i64 x, i64 y) const {
	return get(x, y);
}

template<typename T>
const T& Span2d<T>::get(i64 x, i64 y) const {
	return const_cast<Span2d<T>*>(this)->get(x, y);
}

template<typename T>
T& Span2d<T>::get(i64 x, i64 y) {
	return data_[y * sizeX_ + x];
}

template<typename T>
void Span2d<T>::set(i64 x, i64 y, const T& item) {
	 get(x, y) = item;
}

template<typename T>
i64 Span2d<T>::sizeX() const {
	return sizeX_;
}

template<typename T>
i64 Span2d<T>::sizeY() const {
	return sizeY_;
}

template<typename T>
Vec2T<i64> Span2d<T>::size() const {
	return Vec2T<i64>(sizeX_, sizeY_);
}

template<typename T>
T* Span2d<T>::data() const {
	return data_;
}