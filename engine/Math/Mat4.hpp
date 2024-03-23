#pragma once

#include "Vec4.hpp"
#include "Mat3.hpp"

template<typename T>
struct Mat4T {
	constexpr Mat4T(const Vec4T<T>& x, const Vec4T<T>& y, const Vec4T<T>& z, const Vec4T<T>& w);
	constexpr Mat4T(const Mat3T<T>& m);

	// Using a target instead of direction seems more general. If you already have a direction the code to create a target is more easily readable than the code for creating the direciton if you have position and target.
	static Mat4T translation(const Vec3T<T>& position);
	static Mat4T lookAt(const Vec3T<T>& position, const Vec3T<T>& target, const Vec3T<T>& up);
	static Mat4T perspective(T verticalFov, T aspectRatio, T nearPlaneZ, T farPlaneZ);

	const Vec4T<T>& operator[](i32 i) const;
	Vec4T<T>& operator[](i32 i);
	// Could use for changing the order of access to the data.
	//constexpr T& operator()(i32 x, i32 y);

	Mat4T operator*(const Mat4T& other) const;

	Mat4T inversed() const;
	Mat4T removedTranslation() const;

	const T* data() const;
	T* data();

	static const Mat4T identity;

	Vec4T<T> basis[4];

};

template<typename T>
Vec4T<T> operator*(const Vec4T<T>& v, const Mat4T<T>& m);
template<typename T>
Vec4T<T>& operator*=(Vec4T<T>& v, const Mat4T<T>& m);

using Mat4 = Mat4T<float>;

template<typename T>
constexpr Mat4T<T>::Mat4T(const Vec4T<T>& x, const Vec4T<T>& y, const Vec4T<T>& z, const Vec4T<T>& w)
	: basis{ x, y, z, w } {}

template<typename T>
constexpr Mat4T<T>::Mat4T(const Mat3T<T>& m) 
	: basis{ Vec4T<T>(m[0], 0), Vec4T<T>(m[1], 0), Vec4T<T>(m[2], 0), Vec4T<T>(0, 0, 0, 1) } {
}

template<typename T>
Mat4T<T> Mat4T<T>::translation(const Vec3T<T>& position) {
	return {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ position.x, position.y, position.z, 1},
	};
}

template<typename T>
Mat4T<T> Mat4T<T>::lookAt(const Vec3T<T>& position, const Vec3T<T>& target, const Vec3T<T>& up) {
	const auto forward = (target - position).normalized();
	const auto right = cross(up, forward).normalized();
	const auto cameraUp = cross(forward, right);

	// Translated every object by -position and rotates in the opposite direction to the forward direction. If everything is moved in the reverse direction that it look the same as if the camera was moving to position and rotating to the forward direction.
	// The rotation is transposed, because the inverse of a orthonormal matrix is equal to it's transpose.
	return Mat4T<T>{
		Vec4T<T>(right.x, cameraUp.x, forward.x, 0),
		Vec4T<T>(right.y, cameraUp.y, forward.y, 0),
		Vec4T<T>(right.z, cameraUp.z, forward.z, 0),
		Vec4T<T>(0, 0, 0, 1)
	} * Mat4T<T>::translation(-position);
}

template<typename T>
// nearPlaneZ and farPlaneZ are only used for depth clipping.
Mat4T<T> Mat4T<T>::perspective(T verticalFov, T aspectRatio, T nearPlaneZ, T farPlaneZ) { 
	/*
	// In this comment columns are the basis.

	A basic perspective projection matrix looks like this.
	[1 0 0 0]
	[0 1 0 0]
	[0 0 1 0]
	[0 0 1 0]
	This matrix just copies copies z to w so when the division happens then every point is scaled by 1 / z.
	When a point is divided (by a number > 1) it moves closer to the origin and closer to other points that are also divided. This is what perspective does. Object that are further away appear smaller and the distances between them appear smaller.

	Another parts of the matrix are used to map the coordinates so after division the points inside the frustum get mapped
	to normalized device coordinates, which line inside an AABB from [-1, -1, -1] to [1, 1, 1].

	First mapping the z values.
	[1 0 0 0]
	[0 1 0 0]
	[0 0 a b]
	[0 0 1 0]
	a scales z
	b translates z
	After the matrix multiplication (assuming input.w = 1)
	output.z := a * input.z + b
	and after the division
	output.z := (a * input.z + b) / input.z

	nZ = nearPlaneZ should be mapped to 1 and fZ = farPlaneZ to -1.
	So want this to be satisifed.
	a * nZ + b = nZ
	a * fZ + b = -fZ
	Which after the projection division will make nZ to 1 and fZ to -1. The far plane z is -1 because that is what OpenGL uses.

	Calculating a
	a * nZ + b = nZ
	a * fZ + b = -fZ

	(a * nZ + b) - (a * fZ + b) = nZ - (-fZ)
	a * nZ + b - a * fZ - b = nZ + fZ
	a * nZ - a * fZ = nZ + fZ
	a * (nZ - fZ) = nZ + fZ
	a = (nZ + fZ) / (nZ - fZ)

	Calculating b
	a * nZ + b = nZ
	((nZ + fZ) / (nZ - fZ)) * nZ + b = nZ
	b = nZ - ((nZ + fZ) / (nZ - fZ)) * nZ
	b = ((nZ * (nZ - fZ)) / (nZ - fZ)) - ((nZ + fZ) / (nZ - fZ)) * nZ
	b = ((nZ^2 - nZ * fZ) / (nZ - fZ)) - ((nZ^2 + fZ * nZ) / (nZ - fZ))
	b = ((nZ^2 - nZ * fZ) - (nZ^2 + fZ * nZ)) / (nZ - fZ)
	b = (nZ^2 - nZ * fZ - nZ^2 - fZ * nZ) / (nZ - fZ)
	b = (-nZ * fZ - fZ * nZ) / (nZ - fZ)
	b = (-2 * nZ * fZ) / (nZ - fZ)

	Next mapping the x and y values.

	The the frustum's near rectangle's height is
	tan(verticalFov / 2) * focalLength

	aspectRatio = width / height
	width = height * aspectRatio

	It seems that in many cases the near and far planes are used only for clipping and not for the width of the plane.
	http://learnwebgl.brown37.net/08_projections/projections_perspective.html

	Does this handle the case when z is smaller than 1.

	*/

	/*
	AB   CD
	..___..___
	\        /
	 \      /
	  \____/
	   \  /
	    \/
	The sizes in a perspective projection onto a plane don't depend on the distance to left and right on on the distance forward. 
	If you draw lines to for example to the dotted segments which both have the same length. Then the projection will also have the same length.
	This can be proven using the intercept theorem.
	AB and BC are the same length.
	(AB / BC) and (CD / BC) remain constant at the intercept.
	BC = BC 
	so the length of the projection AB is equal to the length of the projection CD.
	*/

	// focalLength is the distance of the plane onto which the image is projection from the camera position. The implementation of perspective projection I have seen make the focal length independent of the near and far planes and always set it to 1.
	// The near and far planes are only used for depth clipping.
	// Increasing the focal length makes objects appear smaller.
	const auto focalLength = 1;
	const auto imagePlaneHeight = tan(verticalFov / 2) * focalLength;
	const auto imagePlaneWidth = imagePlaneHeight * aspectRatio;

	return {
		{ 1 / imagePlaneWidth, 0, 0, 0 },
		{ 0, 1 / imagePlaneHeight, 0, 0 },
		// The signs are flipeed, because the depth was backwards.
		{ 0, 0, -(nearPlaneZ + farPlaneZ) / (nearPlaneZ - farPlaneZ), 1 },
		{ 0, 0, (2 * nearPlaneZ * farPlaneZ) / (nearPlaneZ - farPlaneZ), 0 },
	};
}

template<typename T>
const Vec4T<T>& Mat4T<T>::operator[](i32 i) const {
	return const_cast<Mat4T<T>*>(this)->operator[](i);
}

template<typename T>
Vec4T<T>& Mat4T<T>::operator[](i32 i) {
	CHECK(i >= 0 && i <= 3);
	return basis[i];
}

template<typename T>
Mat4T<T> Mat4T<T>::operator*(const Mat4T& other) const {
	const auto& m = *this;
	/*return Mat4T(m[0] * other, m[1] * other, m[2] * other, m[3] * other);*/
	return Mat4T(other[0] * m, other[1] * m, other[2] * m, other[3] * m);
}

template<typename T>
Vec4T<T> operator*(const Vec4T<T>& v, const Mat4T<T>& m) {
	return v.x * m[0] + v.y * m[1] + v.z * m[2] + v.w * m[3];
}

template<typename T>
Vec4T<T>& operator*=(Vec4T<T>& v, const Mat4T<T>& m) {
	v = v * m;
	return v;
}

template<typename T>
Mat4T<T> Mat4T<T>::inversed() const {
	T inv[16], det;
	int i;

	const T* m = data();

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	Mat4T<T> inverse = Mat4T<T>(Vec4(), Vec4(), Vec4(), Vec4());

	if (det == 0)
		return inverse;

	det = 1.0 / det;


	for (i = 0; i < 16; i++)
		inverse.data()[i] = inv[i] * det;

	return inverse;




	// https://gamedev.net/forums/topic/464489-quickly-invert-4x4-matrix/4061075/
	// Taken from doom 3 source.

	//Mat2 r0, r1, r2, r3;
	//float a, det, invDet;
	//// Copy
	//Mat4T result = *this;
	//T* mat = result.data();

	//// r0 = m0.Inverse();
	//det = mat[0 * 4 + 0] * mat[1 * 4 + 1] - mat[0 * 4 + 1] * mat[1 * 4 + 0];

	///*if (idMath::Fabs(det) < MATRIX_INVERSE_EPSILON) {
	//	return false;
	//}*/

	//invDet = 1.0f / det;

	//r0[0][0] = mat[1 * 4 + 1] * invDet;
	//r0[0][1] = -mat[0 * 4 + 1] * invDet;
	//r0[1][0] = -mat[1 * 4 + 0] * invDet;
	//r0[1][1] = mat[0 * 4 + 0] * invDet;

	//// r1 = r0 * m1;
	//r1[0][0] = r0[0][0] * mat[0 * 4 + 2] + r0[0][1] * mat[1 * 4 + 2];
	//r1[0][1] = r0[0][0] * mat[0 * 4 + 3] + r0[0][1] * mat[1 * 4 + 3];
	//r1[1][0] = r0[1][0] * mat[0 * 4 + 2] + r0[1][1] * mat[1 * 4 + 2];
	//r1[1][1] = r0[1][0] * mat[0 * 4 + 3] + r0[1][1] * mat[1 * 4 + 3];

	//// r2 = m2 * r1;
	//r2[0][0] = mat[2 * 4 + 0] * r1[0][0] + mat[2 * 4 + 1] * r1[1][0];
	//r2[0][1] = mat[2 * 4 + 0] * r1[0][1] + mat[2 * 4 + 1] * r1[1][1];
	//r2[1][0] = mat[3 * 4 + 0] * r1[0][0] + mat[3 * 4 + 1] * r1[1][0];
	//r2[1][1] = mat[3 * 4 + 0] * r1[0][1] + mat[3 * 4 + 1] * r1[1][1];

	//// r3 = r2 - m3;
	//r3[0][0] = r2[0][0] - mat[2 * 4 + 2];
	//r3[0][1] = r2[0][1] - mat[2 * 4 + 3];
	//r3[1][0] = r2[1][0] - mat[3 * 4 + 2];
	//r3[1][1] = r2[1][1] - mat[3 * 4 + 3];

	//// r3.InverseSelf();
	//det = r3[0][0] * r3[1][1] - r3[0][1] * r3[1][0];

	///*if (idMath::Fabs(det) < MATRIX_INVERSE_EPSILON) {
	//	return false;
	//}*/

	//invDet = 1.0f / det;

	//a = r3[0][0];
	//r3[0][0] = r3[1][1] * invDet;
	//r3[0][1] = -r3[0][1] * invDet;
	//r3[1][0] = -r3[1][0] * invDet;
	//r3[1][1] = a * invDet;

	//// r2 = m2 * r0;
	//r2[0][0] = mat[2 * 4 + 0] * r0[0][0] + mat[2 * 4 + 1] * r0[1][0];
	//r2[0][1] = mat[2 * 4 + 0] * r0[0][1] + mat[2 * 4 + 1] * r0[1][1];
	//r2[1][0] = mat[3 * 4 + 0] * r0[0][0] + mat[3 * 4 + 1] * r0[1][0];
	//r2[1][1] = mat[3 * 4 + 0] * r0[0][1] + mat[3 * 4 + 1] * r0[1][1];

	//// m2 = r3 * r2;
	//mat[2 * 4 + 0] = r3[0][0] * r2[0][0] + r3[0][1] * r2[1][0];
	//mat[2 * 4 + 1] = r3[0][0] * r2[0][1] + r3[0][1] * r2[1][1];
	//mat[3 * 4 + 0] = r3[1][0] * r2[0][0] + r3[1][1] * r2[1][0];
	//mat[3 * 4 + 1] = r3[1][0] * r2[0][1] + r3[1][1] * r2[1][1];

	//// m0 = r0 - r1 * m2;
	//mat[0 * 4 + 0] = r0[0][0] - r1[0][0] * mat[2 * 4 + 0] - r1[0][1] * mat[3 * 4 + 0];
	//mat[0 * 4 + 1] = r0[0][1] - r1[0][0] * mat[2 * 4 + 1] - r1[0][1] * mat[3 * 4 + 1];
	//mat[1 * 4 + 0] = r0[1][0] - r1[1][0] * mat[2 * 4 + 0] - r1[1][1] * mat[3 * 4 + 0];
	//mat[1 * 4 + 1] = r0[1][1] - r1[1][0] * mat[2 * 4 + 1] - r1[1][1] * mat[3 * 4 + 1];

	//// m1 = r1 * r3;
	//mat[0 * 4 + 2] = r1[0][0] * r3[0][0] + r1[0][1] * r3[1][0];
	//mat[0 * 4 + 3] = r1[0][0] * r3[0][1] + r1[0][1] * r3[1][1];
	//mat[1 * 4 + 2] = r1[1][0] * r3[0][0] + r1[1][1] * r3[1][0];
	//mat[1 * 4 + 3] = r1[1][0] * r3[0][1] + r1[1][1] * r3[1][1];

	//// m3 = -r3;
	//mat[2 * 4 + 2] = -r3[0][0];
	//mat[2 * 4 + 3] = -r3[0][1];
	//mat[3 * 4 + 2] = -r3[1][0];
	//mat[3 * 4 + 3] = -r3[1][1];

	////return true;
	//return result;
}

template<typename T>
Mat4T<T> Mat4T<T>::removedTranslation() const {
	auto result = *this;
	result[3] = Vec4T<T>(0.0f, 0.0f, 0.0f, result[3].w);
	return result;
}

template<typename T>
const T* Mat4T<T>::data() const {
	return reinterpret_cast<const float*>(basis);
}

template<typename T>
T* Mat4T<T>::data() {
	return reinterpret_cast<float*>(basis);
}

template<typename T>
const Mat4T<T> Mat4T<T>::identity(Vec4T<T>(1, 0, 0, 0), Vec4T<T>(0, 1, 0, 0), Vec4T<T>(0, 0, 1, 0), Vec4T<T>(0, 0, 0, 1));