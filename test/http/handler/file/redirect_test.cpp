#include <gtest/gtest.h>
#include "http/handler/file/redirect.hpp"
#include "http/request/request.hpp"

namespace http {

TEST(RedirectHandlerTest, RedirectsToDestination) {
    RedirectHandler handler("/new-location");
    
    // Requestオブジェクトの作成に必要な全ての引数を提供
    RawHeaders headers;
    std::vector<char> body;
    
    // ServerContextとLocationContextのポインタ（テスト用にNULLでも可）
    const ServerContext* server = NULL;
    const LocationContext* location = NULL;
    
    Request request(
        kMethodGet,           // HttpMethod method
        "/old-location",      // const std::string &requestTarget
        headers,              // const RawHeaders &headers
        body,                 // const std::vector<char> &body
        server,               // const ServerContext *server
        location              // const LocationContext *location
    );
    
    Either<IAction *, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusFound);
}

} // namespace http
