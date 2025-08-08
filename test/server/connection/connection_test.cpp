
#include <gtest/gtest.h>

#include "Connection.hpp"
#include "ConnectionSocket.hpp"
#include "io/input/reader/reader.hpp"


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

class DummyState : public IConnectionState {
public:
    types::Result<IConnectionState*, int> onEvent() {
        return types::ok<IConnectionState*>(static_cast<IConnectionState*>(this));
    }
};


TEST(ConnectionTest, InitialState) {
    uint16_t port = 65535;
    MockSocketAddr mock = MockSocketAddr("192.168.0.5", port);
    int fd = 9;
    Connection conn(fd, mock);
    EXPECT_EQ(conn.getReadBuffer(), nullptr);
    EXPECT_EQ(conn.getWriteBuffer(), nullptr);
    EXPECT_EQ(conn.getConnState(), nullptr);
    EXPECT_EQ(conn.getConnSock().getRawFd(), fd);
    time_t now = std::time(0);
    EXPECT_LE(std::abs(conn.getLastRecv() - now), 1);
}

TEST(ConnectionTest, AdoptReadBuffer_SetAndReset) {
    uint16_t port = 65535;
    MockSocketAddr mock = MockSocketAddr("192.168.0.5", port);
    int fd = 10;
    Connection conn(fd, mock);
    EXPECT_EQ(conn.getReadBuffer(), static_cast<ReadBuffer*>(0));

    // ReadBuffer を構築（FakeReader でOK）
    FakeReader reader1("hello");
    ReadBuffer* rb1 = new ReadBuffer(reader1);
    conn.adoptReadBuffer(rb1);
    EXPECT_EQ(conn.getReadBuffer(), rb1);

    // 別の ReadBuffer に差し替え（古い方は Connection 側で delete される契約）
    FakeReader reader2("world");
    ReadBuffer* rb2 = new ReadBuffer(reader2);
    conn.adoptReadBuffer(rb2);
    EXPECT_EQ(conn.getReadBuffer(), rb2);

    // reset でポインタがクリアされる
    conn.resetReadBuffer();
    EXPECT_EQ(conn.getReadBuffer(), static_cast<ReadBuffer*>(0));
}

TEST(ConnectionTest, AdoptWriteBuffer_SetAndReset) {
    int fd = 11;
    uint16_t port = 65535;
    MockSocketAddr mock = MockSocketAddr("192.168.0.5", port);
    Connection conn(fd, mock);
    EXPECT_EQ(conn.getWriteBuffer(), static_cast<WriteBuffer*>(0));

    // WriteBuffer がデフォルト構築できない場合は、実際のコンストラクタに合わせて変更
    WriteBuffer* wb1 = new WriteBuffer();
    conn.adoptWriteBuffer(wb1);
    EXPECT_EQ(conn.getWriteBuffer(), wb1);

    WriteBuffer* wb2 = new WriteBuffer();
    conn.adoptWriteBuffer(wb2);
    EXPECT_EQ(conn.getWriteBuffer(), wb2);

    conn.resetWriteBuffer();
    EXPECT_EQ(conn.getWriteBuffer(), static_cast<WriteBuffer*>(0));
}

TEST(ConnectionTest, SetConnState_ReplacesAndDestructorCleansUp) {
    int fd = 12;
    uint16_t port = 65535;
    MockSocketAddr mock = MockSocketAddr("192.168.0.5", port);
    Connection conn(fd, mock);
    EXPECT_EQ(conn.getConnState(), static_cast<IConnectionState*>(0));

    IConnectionState* s1 = new DummyState();
    conn.setConnState(s1);
    EXPECT_EQ(conn.getConnState(), s1);

    IConnectionState* s2 = new DummyState();
    conn.setConnState(s2);
    EXPECT_EQ(conn.getConnState(), s2);

    // ここで明示的に何かを確認するのは難しいが、
    // ・setConnState で古いポインタを delete していること
    // ・~Connection で最終状態を delete していること
    // はクラッシュしないこと＆ASan/LSan を使えば検出可能
}

TEST(ConnectionTest, TimeoutLogic_WorksAroundThreshold) {
    int fd = 13;
    uint16_t port = 65535;
    MockSocketAddr mock = MockSocketAddr("192.168.0.5", port);
    Connection conn(fd, mock);

    const time_t now = std::time(0);

    // 閾値未満 → false
    conn.setLastRecv(now - (60 - 1)); // kTimeoutThresholdSec が 60 前提
    EXPECT_FALSE(conn.isTimeout());

    // 閾値超過 → true
    conn.setLastRecv(now - (60 + 1));
    EXPECT_TRUE(conn.isTimeout());
}
}

