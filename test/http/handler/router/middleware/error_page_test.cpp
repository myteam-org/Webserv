#include <gtest/gtest.h>
#include "http/handler/router/middleware/error_page.hpp"
#include "http/response/response.hpp"
#include "http/request/request.hpp"
#include "config/config.hpp"
#include "action/action.hpp"
#include <vector>
#include "raw_headers.hpp"
#include "config/context/serverContext.hpp"
#include "config/context/locationContext.hpp"

// target を path/query に分割して 9 引数の Request を生成
static http::Request makeRequest(http::HttpMethod m,
                                 const std::string& target,
                                 ServerContext& server,
                                 LocationContext& location) {
    std::string path = target, query;
    if (std::size_t p = target.find('?'); p != std::string::npos) {
        path  = target.substr(0, p);
        query = target.substr(p + 1);
    }
    RawHeaders headers;
    std::vector<char> body;
    return http::Request(m, target, path, query, "HTTP/1.1",
                         headers, body, &server, &location);
}

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
    ServerContext server("server");
    LocationContext location("location");
    http::Request request = makeRequest(http::kMethodGet, "/", server, location);
    MockNextHandler successNextHandler((http::Response(http::kStatusOk)));

    Either<IAction*, http::Response> result = errorPage.intercept(request, successNextHandler);
    EXPECT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), http::kStatusOk);
}

TEST(ErrorPageTest, InterceptError) {
    ErrorPageMap errorPageMap;
    TestErrorPage errorPage(errorPageMap);
    ServerContext server("server");
    LocationContext location("location");
    http::Request request = makeRequest(http::kMethodGet, "/", server, location);
    MockNextHandler errorNextHandler((http::Response(http::kStatusBadRequest)));

    Either<IAction*, http::Response> result = errorPage.intercept(request, errorNextHandler);
    EXPECT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), http::kStatusBadRequest);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
