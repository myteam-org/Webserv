#include "Connection.hpp"
#include <ctime>

Connection::Connection(const ConnectionSocket& connSock)
    : connSock_(connSock),
      readBuffer_(0),
      writeBuffer_(0),
      connState_(0),
      httpRequest_(0),
      lastRecv(std::time(0)) {
}


Connection::~Connection() {
    if (readBuffer_) {
        delete readBuffer_;
        readBuffer_ = 0;
    }
    if (writeBuffer_) {
        delete writeBuffer_;
        writeBuffer_ = 0;
    }
    if (connState_) {
        delete connState_;
        connState_ = 0;
    }
    if (httpRequest_) {
        delete httpRequest_;
        httpRequest_ = 0;
    }
}

const ConnectionSocket& Connection::getConnSock() const {
    return connSock_;
}

ReadBuffer* Connection::getReadBuffer() const {
    return readBuffer_;
}

void Connection::setReadBuffer(ReadBuffer* readBuffer) {
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

void Connection::setWriteBuffer(WriteBuffer* writeBuffer) {
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

http::Request* Connection::getHttpRequest() const {
    return httpRequest_;
}

void Connection::setHttpRequest(http::Request* httpRequest) {
    if (httpRequest_ != httpRequest) {
        if (httpRequest_) {
            delete httpRequest_;
        }
        httpRequest_ = httpRequest;
    }
}

time_t Connection::getLastRecv() const {
    return lastRecv;
}

void Connection::setLastRecv(time_t lastRecvVal) {
    lastRecv = lastRecvVal;
}

bool Connection::isTimeout() {
    time_t now = std::time(0);
    const int timeoutThreshold = 60;
    return (now - lastRecv) > timeoutThreshold;
}
