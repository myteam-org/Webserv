#pragma once
#include "ConnectionSocket.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/write/buffer.hpp"
#include "server/connection/state/IConnectionState.hpp"
#include <ctime>


class Connection {
private:
    ConnectionSocket connSock_;
    ReadBuffer readBuffer_;
    WriteBuffer writeBuffer_;
    IConnectionState* connState_;
    time_t lastRecv_;
    Connection(const Connection&);
    Connection& operator=(const Connection&);
    static const std::time_t kTimeoutThresholdSec = 60;

public:
    Connection (int fd, const ISocketAddr& peerAddr);
    ~Connection();
    const ConnectionSocket& getConnSock() const;
    const ReadBuffer& getReadBuffer() const;
    const WriteBuffer& getWriteBuffer() const;
    ReadBuffer& getReadBuffer();
    WriteBuffer& getWriteBuffer();
    IConnectionState* getConnState() const;
    void setConnState(IConnectionState* connState);
    time_t getLastRecv() const;
    void setLastRecv(time_t lastRecv);
    bool isTimeout() const;
};