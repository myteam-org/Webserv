
#include <gtest/gtest.h>

#include "Connection.hpp"
#include "ConnectionSocket.hpp"
#include "io/input/reader/reader.hpp"
#include <gtest.h>


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

class DummyState : public IConnectionState {
public:
    types::Result<IConnectionState*, int> onEvent() {
        return types::ok<IConnectionState*>(static_cast<IConnectionState*>(this));
    }
};

class FakeReader : public io::IReader {
public:
    FakeReader(const std::string& data) : data_(data), off_(0), eof_(false) {}
    virtual ReadResult read(char* buf, std::size_t nbyte) {
        if (off_ >= data_.size()) {
            eof_ = true;
            return types::ok<std::size_t>(0); // EOF
        }
        std::size_t n = std::min(nbyte, data_.size() - off_);
        std::memcpy(buf, data_.data() + off_, n);
        off_ += n;
        if (off_ >= data_.size()) eof_ = true;
        return types::ok<std::size_t>(n);
    }
    virtual bool eof() { return eof_; }
private:
    std::string data_;
    std::size_t off_;
    bool eof_;
};

TEST(ConnectionTest, InitialState) {
    uint16_t port = 65535;
    MockSocketAddr mock = MockSocketAddr("192.168.0.5", port);
    int fd = 9;
    Connection conn(fd, mock);
    ReadBuffer* rb_addr = &conn.getReadBuffer();
    WriteBuffer* wb_addr = &conn.getWriteBuffer();
    EXPECT_NE(rb_addr, static_cast<ReadBuffer*>(0));
    EXPECT_NE(wb_addr, static_cast<WriteBuffer*>(0));

    // 状態は未設定
    EXPECT_EQ(conn.getConnState(), static_cast<IConnectionState*>(0));

    // ソケットFD
    EXPECT_EQ(conn.getConnSock().getRawFd(), fd);

    // lastRecv_ は現在時刻近傍
    std::time_t now = std::time(0);
    double diff = std::difftime(conn.getLastRecv(), now);
    EXPECT_LE(std::fabs(diff), 1.0);
}
TEST(ConnectionTest, BufferReferencesStableAndConstOverloadsWork) {
    uint16_t port = 65535;
    MockSocketAddr mock("192.168.0.5", port);
    int fd = 10;
    Connection conn(fd, mock);

    // 非const／const の両アクセサが同じ実体を指すことを確認
    ReadBuffer* rb1 = &conn.getReadBuffer();
    const Connection& ccref = conn;
    const ReadBuffer* rb2 = &ccref.getReadBuffer();
    EXPECT_EQ(rb1, rb2);

    WriteBuffer* wb1 = &conn.getWriteBuffer();
    const WriteBuffer* wb2 = &ccref.getWriteBuffer();
    EXPECT_EQ(wb1, wb2);

    // 連続呼び出しでも同じアドレス（直持ちで再配置されない）
    EXPECT_EQ(rb1, &conn.getReadBuffer());
    EXPECT_EQ(wb1, &conn.getWriteBuffer());
}

TEST(ConnectionTest, SetConnState_ReplacesAndDestructorCleansUp) {
    uint16_t port = 65535;
    MockSocketAddr mock("192.168.0.5", port);
    int fd = 12;
    Connection conn(fd, mock);
    EXPECT_EQ(conn.getConnState(), static_cast<IConnectionState*>(0));

    IConnectionState* s1 = new DummyState();
    conn.setConnState(s1);
    EXPECT_EQ(conn.getConnState(), s1);

    IConnectionState* s2 = new DummyState();
    conn.setConnState(s2);
    EXPECT_EQ(conn.getConnState(), s2);

    // ここでは delete の有無はASan/LSanで検出する前提。クラッシュしなければOK。
}

TEST(ConnectionTest, TimeoutLogic_WorksAroundThreshold) {
    uint16_t port = 65535;
    MockSocketAddr mock("192.168.0.5", port);
    int fd = 13;
    Connection conn(fd, mock);

    const std::time_t now = std::time(0);

    // 閾値未満 → false
    conn.setLastRecv(now - (60 - 1)); // kTimeoutThresholdSec = 60 を想定
    EXPECT_FALSE(conn.isTimeout());

    // 閾値超過 → true
    conn.setLastRecv(now - (60 + 1));
    EXPECT_TRUE(conn.isTimeout());
}

} // namespace
