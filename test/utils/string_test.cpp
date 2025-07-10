#include <gtest/gtest.h>
#include "utils/string.hpp"

TEST(StringTest, StartsWith) {
    EXPECT_TRUE(utils::startsWith("hello world", "hello"));
    EXPECT_FALSE(utils::startsWith("hello world", "world"));
    EXPECT_TRUE(utils::startsWith("abc", "abc"));
    EXPECT_FALSE(utils::startsWith("abc", "abcd"));
    EXPECT_TRUE(utils::startsWith("test", ""));
    EXPECT_FALSE(utils::startsWith("", "test"));
    EXPECT_TRUE(utils::startsWith("", ""));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
