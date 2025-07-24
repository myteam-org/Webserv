
#include "http/handler/router/middleware/logger.hpp"
#include "gtest/gtest.h"
#include "http/handler/handler.hpp"
#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "utils/types/either.hpp"
#include "http/method.hpp"
#include "utils/types/option.hpp"
#include "http/status.hpp"

namespace http {

// Dummy IHandler for testing
class DummyHandler : public IHandler {
public:
    // Test function
    Either<IAction*, Response> serve(const Request& request) {
        (void)request;
        Response response(http::kStatusOk, types::none<std::string>(), "");
        return Right<Response>(response);
    }
};

TEST(LoggerTest, Intercept) {
    Logger logger;
    DummyHandler dummy_handler;
    Request request(http::kMethodGet, "/test", "HTTP/1.1", "");

    Either<IAction*, Response> result = logger.intercept(request, dummy_handler);

    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), http::kStatusOk);
}

}
