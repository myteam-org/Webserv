// gtest
#include <gtest/gtest.h>

// 被テスト対象
#include "server/connection/ConnectionManager.hpp"
#include "server/connection/Connection.hpp"
#include "server/socket/ConnectionSocket.hpp"
#include "server/socket/ISocketAddr.hpp"
#include "http/config/config_resolver.hpp"
#include "config/context/serverContext.hpp"

//========================
// Result アダプタ
//========================
#define RESULT_IS_OK(r)   ((r).isOk())
#define RESULT_IS_ERR(r)  ((r).isErr())
#define RESULT_VAL(r)     ((r).unwrap())
#define RESULT_ERR(r)     ((r).unwrapErr())

class DummyAddr : public ISocketAddr {
public:
    DummyAddr() : addr_("127.0.0.1"), port_(8080) {}
    virtual ~DummyAddr() {}
    virtual std::string getAddress() const { return addr_;}
    virtual uint16_t getPort() const { return port_;}
private:
    std::string addr_;
    uint16_t port_;
};

// ConfigResolver のモック実装
class MockConfigResolver : public http::config::IConfigResolver {
public:
    types::Result<const ServerContext*, error::AppError> chooseServer(
        const std::string& /* host */) const {
        static ServerContext dummy("server");
        return types::ok<const ServerContext*>(&dummy);
    }
};

// Connection を new で作る（ConnectionManager が delete する想定のため）
static Connection* makeConn(int fd) {
    static DummyAddr addr;
    static MockConfigResolver resolver;
    return new Connection(fd, addr, resolver);  // 3引数コンストラクタを使用
}

//========================
// テスト本体
//========================

TEST(ConnectionManagerTest, Get_NotFound_ReturnsErr) {
    ConnectionManager mgr;
    types::Result<Connection*, std::string> r = mgr.getConnectionByFd(42);
    EXPECT_TRUE(RESULT_IS_ERR(r));
    EXPECT_NE(RESULT_ERR(r).find("connection not found"), std::string::npos);
}

TEST(ConnectionManagerTest, Register_Valid_Succeeds_And_GetReturnsSamePtr) {
    ConnectionManager mgr;
    const int fd = 10;
    Connection* c = makeConn(fd);

    types::Result<int, int> r1 = mgr.registerConnection(c);
    ASSERT_TRUE(RESULT_IS_OK(r1));
    EXPECT_EQ(RESULT_VAL(r1), fd);

    types::Result<Connection*, std::string> g = mgr.getConnectionByFd(fd);
    ASSERT_TRUE(RESULT_IS_OK(g));
    EXPECT_EQ(RESULT_VAL(g), c);

    // 明示的に unregister して後始末（mgr デストラクタでも解放されるが二重解放はされない設計前提）
    types::Result<int, int> u = mgr.unregisterConnection(fd);
    ASSERT_TRUE(RESULT_IS_OK(u));
    EXPECT_EQ(RESULT_VAL(u), fd);
}

TEST(ConnectionManagerTest, Register_Duplicate_ReturnsErr_And_DoesNotReplaceExisting) {
    ConnectionManager mgr;
    const int fd = 20;
    Connection* c1 = makeConn(fd);
    Connection* c2 = makeConn(fd);  // 同じ fd の別インスタンス

    types::Result<int, int> r1 = mgr.registerConnection(c1);
    ASSERT_TRUE(RESULT_IS_OK(r1));
    EXPECT_EQ(RESULT_VAL(r1), fd);

    types::Result<int, int> r2 = mgr.registerConnection(c2);
    EXPECT_TRUE(RESULT_IS_ERR(r2));     // -2 の想定
    delete c2;

    types::Result<Connection*, std::string> g = mgr.getConnectionByFd(fd);
    ASSERT_TRUE(RESULT_IS_OK(g));
    EXPECT_EQ(RESULT_VAL(g), c1);

    types::Result<int, int> u = mgr.unregisterConnection(fd);
    ASSERT_TRUE(RESULT_IS_OK(u));
    EXPECT_EQ(RESULT_VAL(u), fd);
}

TEST(ConnectionManagerTest, Register_InvalidFd_ReturnsErr) {
    ConnectionManager mgr;
    Connection* bad = makeConn(-1);     // getRawFd() が -1 を返すケース
    types::Result<int, int> r = mgr.registerConnection(bad);
    EXPECT_TRUE(RESULT_IS_ERR(r));      // -1 の想定
    // 採用されていないのでテスト側で破棄
    delete bad;
}

TEST(ConnectionManagerTest, Unregister_NotExisting_ReturnsErr) {
    ConnectionManager mgr;
    types::Result<int, int> r = mgr.unregisterConnection(12345);
    EXPECT_TRUE(RESULT_IS_ERR(r));      // -1 の想定
}

TEST(ConnectionManagerTest, Unregister_Then_Get_NotFound) {
    ConnectionManager mgr;
    const int fd = 30;
    Connection* c = makeConn(fd);

    ASSERT_TRUE(RESULT_IS_OK(mgr.registerConnection(c)));

    types::Result<int, int> u = mgr.unregisterConnection(fd);
    ASSERT_TRUE(RESULT_IS_OK(u));
    EXPECT_EQ(RESULT_VAL(u), fd);
    types::Result<Connection*, std::string> g = mgr.getConnectionByFd(fd);
    EXPECT_TRUE(RESULT_IS_ERR(g));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
