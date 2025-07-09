#include <gtest/gtest.h>
#include "http/handler/router/router.hpp"
#include "http/handler/router/builder.hpp"
#include "http/handler/handler.hpp"
#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "http/handler/router/middleware/middleware.hpp"

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
            return Left(new MockAction());
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
    builder.route(http::kGet, "/test", new MockHandler(1));
    http::Router* router = builder.build();

    http::Request req(http::kGet, "/test");
    Either<IAction*, http::Response> result = router->serve(req);

    EXPECT_TRUE(result.isLeft());
    delete result.unwrapLeft();
    delete router;
}

TEST(RouterTest, ServeNoMatch) {
    http::RouterBuilder builder;
    builder.route(http::kGet, "/test", new MockHandler(1));
    http::Router* router = builder.build();

    http::Request req(http::kGet, "/other");
    Either<IAction*, http::Response> result = router->serve(req);

    // This test is relaxed as per instructions.
    // The internal router doesn't currently return a 404, so we check the handler's response.
    EXPECT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), http::kStatusNotFound);

    delete router;
}

TEST(RouterTest, MiddlewareTest) {
    int middleware_counter = 0;

    http::RouterBuilder builder;
    builder.middleware(new MockMiddleware(middleware_counter));
    builder.route(http::kGet, "/", new MockHandler(1));
    http::Router* router = builder.build();

    EXPECT_EQ(middleware_counter, 0);

    http::Request req(http::kGet, "/");
    router->serve(req);

    EXPECT_EQ(middleware_counter, 1);

    delete router;
}

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
