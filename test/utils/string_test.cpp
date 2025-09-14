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

TEST(StringTest, EndsWith) {
    EXPECT_TRUE(utils::endsWith("hello world", "world"));
    EXPECT_FALSE(utils::endsWith("hello world", "hello"));
    EXPECT_TRUE(utils::endsWith("abc", "abc"));
    EXPECT_FALSE(utils::endsWith("abc", "dabc"));
    EXPECT_TRUE(utils::endsWith("test", ""));
    EXPECT_FALSE(utils::endsWith("", "test"));
    EXPECT_TRUE(utils::endsWith("", ""));
}

TEST(StringTest, ToString) {
    EXPECT_EQ(utils::toString(123), "123");
    EXPECT_EQ(utils::toString(-123), "-123");
    EXPECT_EQ(utils::toString(0), "0");
    EXPECT_EQ(utils::toString(123.456), "123.456");
}

TEST(StringTest, ParseHex) {
    types::Result<std::size_t, error::AppError> result = utils::parseHex("a");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), 10);

    result = utils::parseHex("ff");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), 255);

    result = utils::parseHex("G");
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.unwrapErr(), error::kBadRequest);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}