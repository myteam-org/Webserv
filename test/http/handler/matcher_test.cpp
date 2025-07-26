#include <gtest/gtest.h>
#include "http/handler/matcher.hpp"
#include <string>
#include <map>

TEST(MatcherTest, SimpleMatch) {
    std::map<std::string, int> routes;
    routes["/api"] = 1;
    routes["/api/v1"] = 2;
    http::Matcher<int> matcher(routes);

    types::Option<int> result = matcher.match("/api/v1/users");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), 2);
}

TEST(MatcherTest, ExactMatch) {
    std::map<std::string, int> routes;
    routes["/home"] = 10;
    routes["/home/user"] = 20;
    http::Matcher<int> matcher(routes);

    types::Option<int> result = matcher.match("/home/user");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), 20);
}

TEST(MatcherTest, RootMatch) {
    std::map<std::string, std::string> routes;
    routes["/"] = "root";
    routes["/images/"] = "images";
    http::Matcher<std::string> matcher(routes);

    types::Option<std::string> result = matcher.match("/some/path");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), "root");
}

TEST(MatcherTest, NoMatch) {
    std::map<std::string, int> routes;
    routes["/a"] = 1;
    routes["/b"] = 2;
    http::Matcher<int> matcher(routes);

    types::Option<int> result = matcher.match("/c/d");
    EXPECT_TRUE(result.isNone());
}

TEST(MatcherTest, EmptyInput) {
    std::map<std::string, int> routes;
    routes["/a"] = 1;
    http::Matcher<int> matcher(routes);

    types::Option<int> result = matcher.match("");
    EXPECT_TRUE(result.isNone());
}

TEST(MatcherTest, EmptyRoutes) {
    std::map<std::string, int> routes;
    http::Matcher<int> matcher(routes);

    types::Option<int> result = matcher.match("/any/path");
    EXPECT_TRUE(result.isNone());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
