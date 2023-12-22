#include <gtest/gtest.h>
#include <Span2d.hpp>

TEST(Span2d, CArrayAcess) {
	float data[2][3] = {
		{ 1.0f, 2.0f, 3.0f },
		{ 4.0f, 5.0f, 6.0f }
	};
	
	Span2d span(data);

	EXPECT_EQ(span(0, 0), 1.0f);
	EXPECT_EQ(span(1, 0), 2.0f);
	EXPECT_EQ(span(2, 0), 3.0f);
	EXPECT_EQ(span(0, 1), 4.0f);
	EXPECT_EQ(span(1, 1), 5.0f);
	EXPECT_EQ(span(2, 1), 6.0f);
}