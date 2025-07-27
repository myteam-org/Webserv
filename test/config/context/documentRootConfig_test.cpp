#include <gtest/gtest.h>
#include "config/context/documentRootConfig.hpp"

TEST(DocumentRootConfigTest, IsAutoindexEnabled) {
    DocumentRootConfig config;

    config.setAutoIndex(ON);
    EXPECT_TRUE(config.isAutoindexEnabled());

    config.setAutoIndex(OFF);
    EXPECT_FALSE(config.isAutoindexEnabled());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
