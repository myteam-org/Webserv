#include <gtest/gtest.h>
#include "../src/math.h"

TEST(MathTest, Addition) {
    EXPECT_EQ(add(2, 3), 5);
}

TEST(MathTest, Subtraction) {
    EXPECT_EQ(subtract(5, 3), 2);
}
