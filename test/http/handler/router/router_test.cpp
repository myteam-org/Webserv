#include <gtest/gtest.h>
#include "http/handler/router/router.hpp"
#include "http/handler/router/builder.hpp"
#include "http/handler/handler.hpp"
#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "http/handler/router/middleware/middleware.hpp"
#include <vector>
#include "raw_headers.hpp"
#include "config/context/serverContext.hpp"
#include "config/context/locationContext.hpp"
#include "http/method.hpp"
#include <vector>
#include "raw_headers.hpp"
#include "config/context/serverContext.hpp"
#include "config/context/locationContext.hpp"


// target を path / query に分割
static void splitTarget(const std::string& target,
                        std::string& pathOnly,
                        std::string& queryString) {
    const std::size_t pos = target.find('?');
    if (pos == std::string::npos) {
        pathOnly = target;
        queryString.clear();
    } else {
        pathOnly = target.substr(0, pos);
        queryString = target.substr(pos + 1);
    }
}

// Request を作る（空ヘッダ/空ボディ/HTTP/1.1 をデフォルトで）
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


namespace {

class MockAction : public IAction {
public:
    void execute(ActionContext &ctx) { (void)ctx; }
};

class MockHandler : public http::IHandler {
public:
    explicit MockHandler(int id) : id_(id) {}
    Either<IAction*, http::Response> serve(const http::Request& request) {
        (void)request;
        if (id_ == 1) {
            return Left(static_cast<IAction*>(new MockAction()));
        }
        return Right(http::Response(http::kStatusNotFound, types::some(std::string("Handler not found"))));
    }
private:
    int id_;
};

class MockMiddleware : public http::IMiddleware {
public:
    explicit MockMiddleware(int& counter) : counter_(counter) {}
    Either<IAction*, http::Response> intercept(const http::Request& req, http::IHandler& next) {
        counter_++;
        return next.serve(req);
    }
private:
    int& counter_;
};

TEST(RouterTest, BuildAndServe) {
    http::RouterBuilder builder;
    builder.route(http::kMethodGet, "/test", new MockHandler(1));
    http::Router* router = builder.build();

    // ServerContext / LocationContext を用意
    ServerContext server("server");
    LocationContext location("location");

    // ルートに合わせて「/test」を投げる
    auto request = makeRequest(http::kMethodGet, "/test", server, location);

    Either<IAction*, http::Response> result = router->serve(request); // ← 変数名を修正

    EXPECT_TRUE(result.isLeft());
    delete result.unwrapLeft();
    delete router;
}

TEST(RouterTest, ServeNoMatch) {
    http::RouterBuilder builder;
    builder.route(http::kMethodGet, "/test", new MockHandler(1));
    http::Router* router = builder.build();

    ServerContext server("server");
    LocationContext location("location");

    auto req = makeRequest(http::kMethodGet, "/other", server, location);

    EXPECT_THROW({
        router->serve(req);
    }, std::runtime_error);

    delete router;
}


TEST(RouterTest, MiddlewareTest) {
    int middleware_counter = 0;

    http::RouterBuilder builder;
    builder.middleware(new MockMiddleware(middleware_counter));
    builder.route(http::kMethodGet, "/", new MockHandler(1));
    http::Router* router = builder.build();

    EXPECT_EQ(middleware_counter, 0);

    ServerContext server("server");
    LocationContext location("location");

    auto req = makeRequest(http::kMethodGet, "/", server, location);
    (void)router->serve(req);

    EXPECT_EQ(middleware_counter, 1);

    delete router;
}


}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
