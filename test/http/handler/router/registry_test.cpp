#include <gtest/gtest.h>
#include "http/handler/router/registry.hpp"
#include "http/handler/handler.hpp"
#include "http/response/response.hpp"
#include "http/method.hpp"

using namespace http;

namespace {
class MockHandler : public IHandler {
public:
    explicit MockHandler(int id) : id_(id) {}
    Either<IAction*, Response> serve(const Request& request) {
        (void)request;
        return Right(Response(kStatusOk));
    }
    int id() const { return id_; }
private:
    int id_;
};

class RegistryTest : public ::testing::Test {
protected:
    RouteRegistry registry_;

    void SetUp() override {
        registry_.addRoute(kMethodGet, "/api/v1", new MockHandler(1));
        registry_.addRoute(kMethodPost, "/api/v1", new MockHandler(2));
        registry_.addRoute(kMethodGet, "/", new MockHandler(3));
    }
};

TEST_F(RegistryTest, FindHandler) {
    IHandler* handler = registry_.findHandler(kMethodGet, "/api/v1");
    ASSERT_NE(handler, nullptr);
    EXPECT_EQ(dynamic_cast<MockHandler*>(handler)->id(), 1);

    IHandler* handler2 = registry_.findHandler(kMethodPost, "/api/v1");
    ASSERT_NE(handler2, nullptr);
    EXPECT_EQ(dynamic_cast<MockHandler*>(handler2)->id(), 2);
}

TEST_F(RegistryTest, FindHandlerNoMatch) {
    IHandler* handler = registry_.findHandler(kMethodDelete, "/api/v1");
    EXPECT_EQ(handler, nullptr);

    IHandler* handler2 = registry_.findHandler(kMethodGet, "/api/v2");
    EXPECT_EQ(handler2, nullptr);
}

TEST_F(RegistryTest, MatchPath) {
    types::Option<std::string> path = registry_.matchPath("/api/v1/users");
    ASSERT_TRUE(path.isSome());
    EXPECT_EQ(path.unwrap(), "/api/v1");

    types::Option<std::string> path2 = registry_.matchPath("/");
    ASSERT_TRUE(path2.isSome());
    EXPECT_EQ(path2.unwrap(), "/");
}

TEST_F(RegistryTest, GetAllowedMethods) {
    std::vector<HttpMethod> methods = registry_.getAllowedMethods("/api/v1");
    EXPECT_EQ(methods.size(), 2);

    bool getFound = false;
    bool postFound = false;
    for (size_t i = 0; i < methods.size(); ++i) {
        if (methods[i] == kMethodGet) getFound = true;
        if (methods[i] == kMethodPost) postFound = true;
    }
    EXPECT_TRUE(getFound);
    EXPECT_TRUE(postFound);
}

TEST_F(RegistryTest, AddRouteVector) {
    std::vector<HttpMethod> methods;
    methods.push_back(kMethodPut);
    methods.push_back(kMethodPatch);
    registry_.addRoute(methods, "/api/v2", new MockHandler(4));

    IHandler* handler_put = registry_.findHandler(kMethodPut, "/api/v2");
    ASSERT_NE(handler_put, nullptr);
    EXPECT_EQ(dynamic_cast<MockHandler*>(handler_put)->id(), 4);

    IHandler* handler_patch = registry_.findHandler(kMethodPatch, "/api/v2");
    ASSERT_NE(handler_patch, nullptr);
    EXPECT_EQ(dynamic_cast<MockHandler*>(handler_patch)->id(), 4);
}

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
