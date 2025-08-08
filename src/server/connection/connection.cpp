#include "Connection.hpp"
#include <ctime>

Connection::Connection(int fd, const ISocketAddr& peerAddr)
    : connSock_(fd, peerAddr),
      readBuffer_(0),
      writeBuffer_(0),
      connState_(0),
      lastRecv_(std::time(0)) {
}

Connection::~Connection() {
    if (connState_) {
        delete connState_;
        connState_ = 0;
    }
    resetReadBuffer();
    resetWriteBuffer();
}

const ConnectionSocket& Connection::getConnSock() const {
    return connSock_;
}

ReadBuffer* Connection::getReadBuffer() const {
    return readBuffer_;
}

void Connection::adoptReadBuffer(ReadBuffer* readBuffer) {
    if (readBuffer_ != readBuffer) {
        if (readBuffer_) {
            delete readBuffer_;
        }
        readBuffer_ = readBuffer;
    }
}

WriteBuffer* Connection::getWriteBuffer() const {
    return writeBuffer_;
}

void Connection::adoptWriteBuffer(WriteBuffer* writeBuffer) {
    if (writeBuffer_ != writeBuffer) {
        if (writeBuffer_) {
            delete writeBuffer_;
        }
        writeBuffer_ = writeBuffer;
    }
}

IConnectionState* Connection::getConnState() const {
    return connState_;
}

void Connection::setConnState(IConnectionState* connState) {
    if (connState_ != connState) {
        if (connState_) {
            delete connState_;
        }
        connState_ = connState;
    }
}

time_t Connection::getLastRecv() const {
    return lastRecv_;
}

void Connection::setLastRecv(time_t lastRecvVal) {
    lastRecv_ = lastRecvVal;
}

bool Connection::isTimeout() {
    time_t now = std::time(0);
    return (now - lastRecv_) > kTimeoutThresholdSec;
}

void Connection::resetReadBuffer() {
    if (readBuffer_) {
        delete readBuffer_;
        readBuffer_ = 0;
    }
}

void Connection::resetWriteBuffer() {
    if (writeBuffer_) {
        delete writeBuffer_;
        writeBuffer_ = 0;
    }
}
