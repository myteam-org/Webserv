#include <gtest/gtest.h>
#include "http/request/request.hpp"
#include "http/method.hpp"

using namespace http;

TEST(RequestTest, ConstructorAndGetters) {
    Request req(kMethodGet, "/index.html", "HTTP/1.1", "body content");
    EXPECT_EQ(req.getMethod(), kMethodGet);
    EXPECT_EQ(req.getRequestTarget(), "/index.html");
    EXPECT_EQ(req.getHttpVersion(), "HTTP/1.1");
    EXPECT_EQ(req.getBody(), "body content");
}

TEST(RequestTest, ConstructorWithDefaults) {
    Request req(kMethodPost, "/api/users");
    EXPECT_EQ(req.getMethod(), kMethodPost);
    EXPECT_EQ(req.getRequestTarget(), "/api/users");
    EXPECT_EQ(req.getHttpVersion(), "HTTP/1.1"); // Default value
    EXPECT_EQ(req.getBody(), ""); // Default value
}

TEST(RequestTest, EqualityOperator) {
    Request req1(kMethodGet, "/test", "HTTP/1.1", "");
    Request req2(kMethodGet, "/test", "HTTP/1.1", "");
    Request req3(kMethodPost, "/test", "HTTP/1.1", "");
    Request req4(kMethodGet, "/other", "HTTP/1.1", "");
    Request req5(kMethodGet, "/test", "HTTP/1.0", "");
    Request req6(kMethodGet, "/test", "HTTP/1.1", "body");

    EXPECT_TRUE(req1 == req2);
    EXPECT_FALSE(req1 == req3);
    EXPECT_FALSE(req1 == req4);
    EXPECT_FALSE(req1 == req5);
    EXPECT_FALSE(req1 == req6);
}

TEST(RequestTest, GetHeader) {
    Request req(kMethodGet, "/", "HTTP/1.1", "");
    types::Option<std::string> header = req.getHeader("Host");
    EXPECT_TRUE(header.isNone()); // Currently returns None
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
