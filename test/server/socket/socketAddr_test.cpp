#include <gtest/gtest.h>
#include "server/socket/SocketAddr.hpp"

namespace {

TEST(SocketAddrTest, CreateIPv4AndGetters) {

    std::string ip = "127.0.0.1";
    uint16_t port = 8080;
    SocketAddr addr = SocketAddr::createIPv4(ip, port);

    EXPECT_EQ(addr.getAddress(), ip);
    EXPECT_EQ(addr.getPort(), port);
    EXPECT_EQ(addr.length(), sizeof(sockaddr_in));

    sockaddr_in* rawAddr = reinterpret_cast<sockaddr_in*>(addr.raw());
    EXPECT_EQ(ntohs(rawAddr->sin_port), port);
    EXPECT_EQ(rawAddr->sin_family, AF_INET);
}

TEST(SocketAddrTest, CreateIPv4AndGetters2) {
    std::string ip = "localhost";
    uint16_t port = 8080;
    SocketAddr addr = SocketAddr::createIPv4(ip, port);

    EXPECT_EQ(addr.getAddress(), "127.0.0.1");
    EXPECT_EQ(addr.getPort(), port);
    EXPECT_EQ(addr.length(), sizeof(sockaddr_in));

    sockaddr_in* rawAddr = reinterpret_cast<sockaddr_in*>(addr.raw());
    EXPECT_EQ(ntohs(rawAddr->sin_port), port);
    EXPECT_EQ(rawAddr->sin_family, AF_INET);
}

TEST(SocketAddrTest, ResolveByNameInvalidHost) {
    EXPECT_THROW({
        SocketAddr::createIPv4("invalid-host-name.example", 8080);
    }, std::runtime_error);
}

TEST(SocketAddrTest, RawPointerAndLength) {
    SocketAddr addr = SocketAddr::createIPv4("192.168.0.1", 1234);

    sockaddr* raw = addr.raw();
    EXPECT_NE(raw, static_cast<sockaddr*>(NULL));
    EXPECT_EQ(addr.length(), sizeof(sockaddr_in));

    addr.setLength(100);
    EXPECT_EQ(addr.length(), 100);
}


}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
