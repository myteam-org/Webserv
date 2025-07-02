#include <gtest/gtest.h>
#include "http/method.hpp"

namespace {

TEST(HttpMethodTest, FromString_KnownMethods) {
    EXPECT_EQ(http::httpMethodFromString("GET"), http::kMethodGet);
    EXPECT_EQ(http::httpMethodFromString("POST"), http::kMethodPost);
    EXPECT_EQ(http::httpMethodFromString("PUT"), http::kMethodPut);
    EXPECT_EQ(http::httpMethodFromString("DELETE"), http::kMethodDelete);
    EXPECT_EQ(http::httpMethodFromString("HEAD"), http::kMethodHead);
    EXPECT_EQ(http::httpMethodFromString("OPTIONS"), http::kMethodOptions);
    EXPECT_EQ(http::httpMethodFromString("TRACE"), http::kMethodTrace);
    EXPECT_EQ(http::httpMethodFromString("CONNECT"), http::kMethodConnect);
    EXPECT_EQ(http::httpMethodFromString("PATCH"), http::kMethodPatch);
}

TEST(HttpMethodTest, FromString_UnknownMethod) {
    EXPECT_EQ(http::httpMethodFromString("FOO"), http::kMethodUnknown);
    EXPECT_EQ(http::httpMethodFromString(""), http::kMethodUnknown);
    EXPECT_EQ(http::httpMethodFromString("get"), http::kMethodUnknown); // case-sensitive
}

TEST(HttpMethodTest, ToString_KnownMethods) {
    EXPECT_EQ(http::httpMethodToString(http::kMethodGet), "GET");
    EXPECT_EQ(http::httpMethodToString(http::kMethodPost), "POST");
    EXPECT_EQ(http::httpMethodToString(http::kMethodPut), "PUT");
    EXPECT_EQ(http::httpMethodToString(http::kMethodDelete), "DELETE");
    EXPECT_EQ(http::httpMethodToString(http::kMethodHead), "HEAD");
    EXPECT_EQ(http::httpMethodToString(http::kMethodOptions), "OPTIONS");
    EXPECT_EQ(http::httpMethodToString(http::kMethodTrace), "TRACE");
    EXPECT_EQ(http::httpMethodToString(http::kMethodConnect), "CONNECT");
    EXPECT_EQ(http::httpMethodToString(http::kMethodPatch), "PATCH");
}

TEST(HttpMethodTest, ToString_UnknownMethod) {
    EXPECT_EQ(http::httpMethodToString(http::kMethodUnknown), "UNKNOWN");
    // 範囲外の値でも"UNKNOWN"を返すことを期待
    EXPECT_EQ(http::httpMethodToString(static_cast<http::HttpMethod>(123)), "UNKNOWN");
}

TEST(HttpMethodTest, RoundTrip_KnownMethods) {
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodGet)), http::kMethodGet);
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodPost)), http::kMethodPost);
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodPut)), http::kMethodPut);
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodDelete)), http::kMethodDelete);
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodHead)), http::kMethodHead);
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodOptions)), http::kMethodOptions);
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodTrace)), http::kMethodTrace);
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodConnect)), http::kMethodConnect);
    EXPECT_EQ(http::httpMethodFromString(http::httpMethodToString(http::kMethodPatch)), http::kMethodPatch);
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
