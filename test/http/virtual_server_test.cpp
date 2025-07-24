#include <gtest/gtest.h>
#include "http/virtual_server.hpp"
#include "config/context/serverContext.hpp"

TEST(VirtualServerTest, Constructor) {
    ServerContext serverContext("");
    VirtualServer virtualServer(serverContext, "127.0.0.1");
    EXPECT_EQ(virtualServer.getServerConfig().getValue(), serverContext.getValue());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
