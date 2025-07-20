#include <gtest/gtest.h>
#include "http/status.hpp"
#include "utils/types/option.hpp"

TEST(HttpStatusTest, httpStatusCodeFromInt) {
    types::Option<http::HttpStatusCode> code = http::httpStatusCodeFromInt(200);
    EXPECT_TRUE(code.isSome());
    EXPECT_EQ(code.unwrap(), http::kStatusOk);

    types::Option<http::HttpStatusCode> invalid_code = http::httpStatusCodeFromInt(999);
    EXPECT_TRUE(invalid_code.isNone());
}

TEST(HttpStatusTest, getHttpStatusText) {
    EXPECT_EQ(http::getHttpStatusText(http::kStatusOk), "OK");
    EXPECT_EQ(http::getHttpStatusText(http::kStatusNotFound), "Not Found");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
