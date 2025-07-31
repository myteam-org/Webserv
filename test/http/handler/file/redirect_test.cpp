#include <gtest/gtest.h>
#include "http/handler/file/redirect.hpp"
#include "http/request/request.hpp"

namespace http {

TEST(RedirectHandlerTest, RedirectsToDestination) {
    RedirectHandler handler("/new-location");
    Request request(kMethodGet, "/old-location");

    Either<IAction *, Response> result = handler.serve(request);

    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusFound);
}

} // namespace http
