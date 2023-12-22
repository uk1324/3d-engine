#include <gtest/gtest.h>
#include <engine/Math/Matrix/Arithmetic.hpp>
#include <Span2d.hpp>

const auto SIZE_X = 4;
const auto SIZE_Y = 3;

float aData[SIZE_X][SIZE_Y]{
	{ 1.0f, 2.0f, 3.0f },
	{ 0.5f, 0.25f, 0.5f },
	{ 1.0f, 2.0f, 3.0f },
	{ 1000.0f, 10000.0f, 10000.0f },
};
Span2d<float> a(aData);

float bData[SIZE_X][SIZE_Y]{
	{ 0.25f, 3.0f, 1.0f },
	{ 100.0f, 3811.0f, 3.75f },
	{ 100.0f, 38.125f, 3.75f },
	{ 0.0f, 12.0f, 7.0f },
};
Span2d<float> b(bData);

float outputData[SIZE_X][SIZE_Y];
Span2d<float> output(outputData);

TEST(MatrixArithmetic, Addition) {
	float expectedData[SIZE_X][SIZE_Y]{
		{ 1.25f, 5.0f, 4.0f },
		{ 100.5f, 3811.25f, 4.25f },
		{ 101.0f, 40.125f, 6.75f },
		{ 1000.0f, 10012.0f, 10007.0f },
	};
	Span2d<float> expected(expectedData);

	Mat::add<float>(output, a, b);
	ASSERT_EQ(output, expected);
}

TEST(MatrixArithmetic, Multiplication) {
	float aData[3][2]{
		{ 1.0f, 2.0f },
		{ 3.0f, 4.0f },
		{ 5.0f, 6.0f },
	};
	Span2d<float> a(aData);

	float bData[2][2]{
		{ 7.0f, 8.0f },
		{ 9.0f, 10.0f },
	};
	Span2d<float> b(bData);

	float expectedData[3][2]{
		{ 25.0f, 28.0f },
		{ 57.0f, 64.0f },
		{ 89.0f, 100.0f }
	};
	Span2d expected(expectedData);

	float outputData[3][2];
	Span2d output(outputData);

	Mat::multiply<float>(output, a, b);
	ASSERT_EQ(output, expected);
}