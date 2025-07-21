#include <gtest/gtest.h>
#include "http/handler/router/middleware/error_page.hpp"
#include "http/response/response.hpp"
#include "http/request/request.hpp"
#include "config/config.hpp"
#include "action/action.hpp"

class MockNextHandler : public http::IHandler {
public:
    MockNextHandler(const http::Response& response) : response_(response) {}

    Either<IAction*, http::Response> serve(const http::Request& request) {
        (void)request;
        return types::Right<http::Response>(response_);
    }
private:
    http::Response response_;
};

class TestErrorPage : public http::ErrorPage {
public:
    TestErrorPage(const ErrorPageMap& errorPageMap) : http::ErrorPage(errorPageMap) {}
    Either<IAction*, http::Response> serve(const http::Request& request) {
        (void)request;
        return types::Right<http::Response>(http::Response(http::kStatusNotImplemented));
    }
};

TEST(ErrorPageTest, InterceptSuccess) {
    ErrorPageMap errorPageMap;
    TestErrorPage errorPage(errorPageMap);
    http::Request request(http::kMethodGet, "/", "HTTP/1.1", "");
    MockNextHandler successNextHandler(http::Response(http::kStatusOk));

    Either<IAction*, http::Response> result = errorPage.intercept(request, successNextHandler);
    EXPECT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), http::kStatusOk);
}

TEST(ErrorPageTest, InterceptError) {
    ErrorPageMap errorPageMap;
    TestErrorPage errorPage(errorPageMap);
    http::Request request(http::kMethodGet, "/", "HTTP/1.1", "");
    MockNextHandler errorNextHandler(http::Response(http::kStatusBadRequest));

    Either<IAction*, http::Response> result = errorPage.intercept(request, errorNextHandler);
    EXPECT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), http::kStatusBadRequest);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}