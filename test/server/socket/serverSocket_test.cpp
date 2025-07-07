#include <gtest/gtest.h>
#include "server/socket/ServerSocket.hpp"
#include "server/socket/ConnectionSocket.hpp"
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
// テスト対象：ServerSocket
namespace {

const uint16_t kTestPort = 55557;
const std::string kTestAddress = "127.0.0.1";


extern "C" void* clientThreadFunc(void* arg) {
    sleep(1);  // Server preparation wait

    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(kTestPort);
    inet_pton(AF_INET, kTestAddress.c_str(), &serverAddr.sin_addr);

    connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    close(sock);

    return NULL;
}

TEST(ServerSocketTest, ConstructAndBindListen) {
    EXPECT_NO_THROW({
        ServerSocket server(kTestPort, kTestAddress);

        EXPECT_EQ(server.getBindPort(), kTestPort);
        EXPECT_EQ(server.getBindAddress(), kTestAddress);
        EXPECT_NE(server.getRawFd(), -1);
    });
}

TEST(ServerSocketTest, ListenTwiceFails) {
    ServerSocket server(kTestPort, kTestAddress);
    try {
        ServerSocket server2(kTestPort, kTestAddress);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        std::cerr << "Error in ListenTwiceFails: " << e.what() << std::endl;
    }
}

TEST(ServerSocketTest, InvalidHostNameThrows) {
    EXPECT_THROW({
        ServerSocket server(kTestPort, "invalid_host_name");
    }, std::runtime_error);
}

TEST(ServerSocketTest, AcceptConnection) {
    // Setup server
    try {
        ServerSocket server(kTestPort, kTestAddress);


    // Start client thread
    pthread_t clientThread;
    ASSERT_EQ(pthread_create(&clientThread, NULL, clientThreadFunc, NULL), 0);

    // Accept the client connection
    ServerSocket::ConnectionResult result = server.accept();

    ASSERT_TRUE(result.isOk());
    ConnectionSocket* conn = result.unwrap();

    // Verify the connection socket
    EXPECT_NE(conn->getRawFd(), -1);
    EXPECT_EQ(conn->getPeerAddress(), kTestAddress);

    delete conn;
    pthread_join(clientThread, NULL);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error in AcceptConnection: " << e.what() << std::endl;
        FAIL();
    }
}

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
