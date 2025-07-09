#include <gtest/gtest.h>
#include "utils/types/either.hpp"
#include <string>

TEST(EitherTest, IsLeft) {
    Either<int, std::string> either = Left(10);
    EXPECT_TRUE(either.isLeft());
    EXPECT_FALSE(either.isRight());
}

TEST(EitherTest, IsRight) {
    Either<int, std::string> either = Right(std::string("hello"));
    EXPECT_FALSE(either.isLeft());
    EXPECT_TRUE(either.isRight());
}

TEST(EitherTest, UnwrapLeft) {
    Either<int, std::string> either = Left(10);
    EXPECT_EQ(either.unwrapLeft(), 10);
}

TEST(EitherTest, UnwrapLeftOnRight) {
    Either<int, std::string> either = Right(std::string("hello"));
    EXPECT_THROW(either.unwrapLeft(), std::runtime_error);
}

TEST(EitherTest, UnwrapRight) {
    Either<int, std::string> either = Right(std::string("hello"));
    EXPECT_EQ(either.unwrapRight(), "hello");
}

TEST(EitherTest, UnwrapRightOnLeft) {
    Either<int, std::string> either = Left(10);
    EXPECT_THROW(either.unwrapRight(), std::runtime_error);
}

TEST(EitherTest, CopyConstructor) {
    Either<int, std::string> either1 = Left(20);
    Either<int, std::string> either2 = either1;
    EXPECT_TRUE(either2.isLeft());
    EXPECT_EQ(either2.unwrapLeft(), 20);

    Either<int, std::string> either3 = Right(std::string("world"));
    Either<int, std::string> either4 = either3;
    EXPECT_TRUE(either4.isRight());
    EXPECT_EQ(either4.unwrapRight(), "world");
}

TEST(EitherTest, AssignmentOperator) {
    Either<int, std::string> either1 = Left(30);
    Either<int, std::string> either2 = Right(std::string("tmp"));
    either2 = either1;
    EXPECT_TRUE(either2.isLeft());
    EXPECT_EQ(either2.unwrapLeft(), 30);

    Either<int, std::string> either3 = Right(std::string("assign"));
    either1 = either3;
    EXPECT_TRUE(either1.isRight());
    EXPECT_EQ(either1.unwrapRight(), "assign");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
