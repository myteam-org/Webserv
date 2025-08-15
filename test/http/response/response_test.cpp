#include <gtest/gtest.h>
#include "http/response/response.hpp"
#include "http/status.hpp"
#include "option.hpp"

using namespace http;

TEST(ResponseTest, ConstructorAndGetters) {
    Response res(kStatusOk, types::some(std::string("Hello")), "HTTP/1.1");
    EXPECT_EQ(res.getStatusCode(), kStatusOk);
    EXPECT_EQ(res.getBody().unwrap(), "Hello");
    EXPECT_EQ(res.getHttpVersion(), "HTTP/1.1");
}

TEST(ResponseTest, ConstructorWithDefaultBody) {
    Response res(kStatusNotFound);
    EXPECT_EQ(res.getStatusCode(), kStatusNotFound);
    EXPECT_TRUE(res.getBody().isNone());
    EXPECT_EQ(res.getHttpVersion(), "HTTP/1.1");
}

TEST(ResponseTest, EqualityOperator) {
    Response res1(kStatusOk, types::some(std::string("Body1")), "HTTP/1.1");
    Response res2(kStatusOk, types::some(std::string("Body1")), "HTTP/1.1");
    Response res3(kStatusNotFound, types::some(std::string("Body1")), "HTTP/1.1");
    Response res4(kStatusOk, types::some(std::string("Body2")), "HTTP/1.1");
    Response res5(kStatusOk, types::some(std::string("Body1")), "HTTP/1.0");

    EXPECT_TRUE(res1 == res2);
    EXPECT_FALSE(res1 == res3);
    EXPECT_FALSE(res1 == res4);
    EXPECT_FALSE(res1 == res5);

    Response res6(kStatusOk);
    Response res7(kStatusOk);
    EXPECT_TRUE(res6 == res7);
    EXPECT_FALSE(res1 == res6);
}

TEST(ResponseTest, ToString) {
    Response res(kStatusOk);
    const std::string s = res.toString();
    EXPECT_TRUE(s.find("HTTP/1.1 200 OK\r\n") == 0);
    // EXPECT_EQ(res.toString(), ""); // Current implementation returns empty string
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
