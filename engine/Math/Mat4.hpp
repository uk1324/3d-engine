#pragma once

#include "Vec4.hpp"

template<typename T>
struct Mat4T {
	constexpr Mat4T(const Vec4T<T>& x, const Vec4T<T>& y, const Vec4T<T>& z, const Vec4T<T>& w);

	// Using a target instead of direction seems more general. If you already have a direction the code to create a target is more easily readable than the code for creating the direciton if you have position and target.
	static Mat4T translation(const Vec3T<T>& position);
	static Mat4T lookAt(const Vec3T<T>& position, const Vec3T<T>& target, const Vec3T<T>& up);
	static Mat4T perspective(T verticalFov, T aspectRatio, T nearPlaneZ, T farPlaneZ);

	const Vec4T<T>& operator[](i32 i) const;
	Vec4T<T>& operator[](i32 i);
	// Could use for changing the order of access to the data.
	//constexpr T& operator()(i32 x, i32 y);

	Mat4T operator*(const Mat4T& other) const;

	const T* data();

	static const Mat4T identity;

	Vec4T<T> basis[4];

};

using Mat4 = Mat4T<float>;

template<typename T>
constexpr Mat4T<T>::Mat4T(const Vec4T<T>& x, const Vec4T<T>& y, const Vec4T<T>& z, const Vec4T<T>& w)
	: basis{ x, y, z, w} {}

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
	return Mat4T<T>::translation(-position) * Mat4T<T>{
		Vec4T<T>(right.x, cameraUp.x, forward.x, 0),
		Vec4T<T>(right.y, cameraUp.y, forward.y, 0),
		Vec4T<T>(right.z, cameraUp.z, forward.z, 0),
		Vec4T<T>(0, 0, 0, 1)
	};
}

template<typename T>
Mat4T<T> Mat4T<T>::perspective(T verticalFov, T aspectRatio, T nearPlaneZ, T farPlaneZ) {
	//GenericMat4<T> mat;

	/*
	// Columns are the basis.

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
	Which after the projection division will make nZ to 1 and fZ to -1. (TODO: I think the signs might be swapped thats why the output sign is wrong Or maybe just don't flip the z in the transform?).

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

	const auto focalLength = 1;
	const auto imagePlaneHeight = tan(verticalFov / 2) * focalLength;
	const auto imagePlaneWidth = imagePlaneHeight * aspectRatio;

	// tan(fov) = farPlaneHalfHeight / distanceToFarPlane
	// when the matrix is applied then y' = y * (farPlaneHalfHeight / distanceToFarPlane)

	// Do things behind the focal plane also get projected onto it.
	//mat(1, 1) = halfFovTan;
	//mat(0, 0) = halfFovTan / aspectRatio;
	//mat(2, 2) = (nearZ + farZ) / (nearZ - farZ);
	//// Because Vec3::forward = [0, 0, 1] and OpenGL forward is [0, 0, -1] the z axis and the translation on it has to be flipped.
	//mat(2, 3) = T(-1);
	//mat(3, 2) = (T(2) * farZ * nearZ) / (nearZ - farZ);

	/*return {
		{ 1 / imagePlaneWidth, 0, 0, 0 },
		{ 0, 1 / imagePlaneHeight, 0, 0 },
		{ 0, 0, (nearPlaneZ + farPlaneZ) / (nearPlaneZ - farPlaneZ), 1 },
		{ 0, 0, (-2 * nearPlaneZ * farPlaneZ) / (nearPlaneZ - farPlaneZ), 0 },
	};*/

	//T halfFovTan = tan(verticalFov / 2);
	//// tan(fov) = farPlaneHalfHeight / distanceToFarPlane
	//// when the matrix is applied then y' = y * (farPlaneHalfHeight / distanceToFarPlane)
	//const auto nearZ = nearPlaneZ;
	//const auto farZ = farPlaneZ;
	//Mat4T<T> mat = Mat4T::identity;
	//mat[0][0] = halfFovTan / aspectRatio;
	//mat[1][1] = halfFovTan;
	//mat[2][2] = (nearZ + farZ) / (nearZ - farZ);
	//mat[2][3] = T(-1);
	//mat[3][2] = (T(2) * farZ * nearZ) / (nearZ - farZ);

	/*const auto far = farPlaneZ;
	const auto near = nearPlaneZ;
	const auto fov = verticalFov;
	T fovInv = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);
	Mat4T<T> mat = Mat4T<T>::identity;
	mat[0][0] = fovInv;
	mat[1][1] = aspectRatio * fovInv;
	mat[2][2] = far / (far - near);
	mat[3][2] = (-far * near) / (far - near);
	mat[2][3] = 1.0f;
	mat[3][3] = 0.0f;*/

	/*const auto far = farPlaneZ;
	const auto near = nearPlaneZ;
	const auto fov = verticalFov;
	T fovInv = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);
	Mat4T<T> mat = Mat4T<T>::identity;
	mat[0][0] = fovInv;
	mat[1][1] = aspectRatio * fovInv;
	mat[2][2] = far / (far - near);
	mat[3][2] = 2 * (-far * near) / (far - near);
	mat[2][3] = 1.0f;
	mat[3][3] = 0.0f;*/

	/*
	AB   CD
	..___..___
	\        /
	 \      /
	  \____/
	   \  /
	    \/
	The sizes in a perspective projection onto a plane don't depend on the distance to left and right. 
	If you draw lines to for example to the dotted segments which both have the same length. Then the projection will also have the same length.
	This can be proven using the intercept theorem.
	AB and BC are the same length.
	(AB / BC) and (CD / BC) remain constant at the intercept.
	BC = BC 
	so the length of the projection AB is equal to the length of the projection CD.
	*/

	const auto far = farPlaneZ;
	const auto near = nearPlaneZ;
	const auto nZ = near;
	const auto fZ = far;
	const auto fov = verticalFov;
	//T fovInv = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);
	//Mat4T<T> mat = Mat4T<T>::identity;
	//mat[0][0] = fovInv;
	//mat[1][1] = aspectRatio * fovInv;
	//mat[2][2] = (nZ + fZ) / (nZ - fZ);
	////mat[3][2] = 2 * (-far * near) / (far - near);
	//mat[3][2] = (-2 * nZ * fZ) / (nZ - fZ);
	//mat[2][3] = 1.0f;
	//mat[3][3] = 0.0f;

	T fovInv = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);
	//Mat4T<T> mat = Mat4T<T>::identity;
	//mat[0][0] = fovInv;
	//mat[1][1] = aspectRatio * fovInv;
	//mat[2][2] = (nZ + fZ) / (nZ - fZ);
	////mat[3][2] = 2 * (-far * near) / (far - near);
	//mat[3][2] = (-2 * nZ * fZ) / (nZ - fZ);
	//mat[2][3] = 1.0f;
	//mat[3][3] = 0.0f;

	return {
		{ fovInv, 0, 0, 0 },
		{ 0, aspectRatio * fovInv, 0, 0 },
		{ 0, 0, (nZ + fZ) / (nZ - fZ), 1 },
		{ 0, 0, (-2 * nZ * fZ) / (nZ - fZ), 0 },
	};
}

template<typename T>
const Vec4T<T>& Mat4T<T>::operator[](i32 i) const {
	return const_cast<Mat4T<T>*>(this)->operator[](i);
}

template<typename T>
Vec4T<T>& Mat4T<T>::operator[](i32 i) {
	CHECK(i >= 0 && i <= 4);
	return basis[i];
}

template<typename T>
Mat4T<T> Mat4T<T>::operator*(const Mat4T& other) const {
	const auto& m = *this;
	return Mat4T(m[0] * other, m[1] * other, m[2] * other, m[3] * other);
}

template<typename T>
Vec4T<T> operator*(const Vec4T<T>& v, const Mat4T<T>& m);

template<typename T>
Vec4T<T> operator*(const Vec4T<T>& v, const Mat4T<T>& m) {
	return v.x * m[0] + v.y * m[1] + v.z * m[2] + v.w * m[3];
}

template<typename T>
const T* Mat4T<T>::data() {
	return reinterpret_cast<float*>(basis);
}

template<typename T>
const Mat4T<T> Mat4T<T>::identity(Vec4T<T>(1, 0, 0, 0), Vec4T<T>(0, 1, 0, 0), Vec4T<T>(0, 0, 1, 0), Vec4T<T>(0, 0, 0, 1));