#include <gtest/gtest.h>
#include "server/socket/ConnectionSocket.hpp"
#include "server/socket/SocketAddr.hpp"
#include "io/base/FileDescriptor.hpp"

namespace {

class MockSocketAddr : public ISocketAddr {
public:
    MockSocketAddr(const std::string& ip, uint16_t port) : ip_(ip), port_(port) {}
    std::string getAddress() const { return ip_ ;}
    uint16_t getPort() const { return port_ ; }
private:
    std::string ip_;
    uint16_t port_;
};

TEST(ConnectionSocketTest, BasicInfoTest) {
    uint16_t port = 65535;
    MockSocketAddr mock = MockSocketAddr("192.168.0.5", port);
    int fd = 42;
    ConnectionSocket testConnetionSocket(fd, mock);
    EXPECT_EQ(testConnetionSocket.getRawFd(), fd);
    EXPECT_EQ(testConnetionSocket.getPeerAddress(), "192.168.0.5");
    EXPECT_EQ(testConnetionSocket.getPeerPort(), port);
    EXPECT_EQ(testConnetionSocket.getClientInfo(), "192.168.0.5:65535");

}
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
