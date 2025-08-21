
#include "http/handler/router/middleware/logger.hpp"

#include <vector>

#include "config/context/locationContext.hpp"
#include "config/context/serverContext.hpp"
#include "gtest/gtest.h"
#include "http/handler/handler.hpp"
#include "http/method.hpp"
#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "http/status.hpp"
#include "raw_headers.hpp"
#include "utils/types/either.hpp"
#include "utils/types/option.hpp"

// target を path/query に分割して 9 引数の Request を生成
static http::Request makeRequest(http::HttpMethod m, const std::string& target,
                                 ServerContext& server,
                                 LocationContext& location) {
    std::string pathOnly = target;
    std::string queryString;
    std::size_t pos = target.find('?');
    if (pos != std::string::npos) {
        pathOnly = target.substr(0, pos);
        queryString = target.substr(pos + 1);
    }

    RawHeaders headers;
    std::vector<char> body;

    return http::Request(
        m,
        /*requestTarget*/ target,
        /*pathOnly*/      pathOnly,
        /*queryString*/   queryString,
        /*headers*/       headers,
        /*body*/          body,
        /*server*/        &server,
        /*location*/      &location
    );
}
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
    ServerContext server("server");
    LocationContext location("location");
    auto request = makeRequest(http::kMethodGet, "/test", server, location);

    Either<IAction*, Response> result =
        logger.intercept(request, dummy_handler);

    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), http::kStatusOk);
}

}  // namespace http
