#include "server/connection/Connection.hpp"
#include <ctime>
#include "io/input/reader/fd.hpp"

Connection::Connection(int fd, const ISocketAddr& peerAddr)
    : connSock_(fd, peerAddr),
      connState_(0),
      readBuffer_(connSock_),
      writeBuffer_(connSock_),
      lastRecv_(std::time(0)) {}

Connection::~Connection() {
    if (connState_) {
        delete connState_;
        connState_ = 0;
    }
}

const ConnectionSocket& Connection::getConnSock() const {
    return connSock_;
}

ReadBuffer& Connection::getReadBuffer() {
    return readBuffer_;
}
const ReadBuffer& Connection::getReadBuffer() const {
    return readBuffer_;
}

WriteBuffer& Connection::getWriteBuffer() {
    return writeBuffer_;
}

const WriteBuffer& Connection::getWriteBuffer() const {
    return writeBuffer_;
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

bool Connection::isTimeout() const{
    time_t now = std::time(0);
    return (now - lastRecv_) > kTimeoutThresholdSec;
}

// void Connection::adoptReadBuffer(ReadBuffer* readBuffer) {
//     if (readBuffer_ != readBuffer) {
//         if (readBuffer_) {
//             delete readBuffer_;
//         }
//         readBuffer_ = readBuffer;
//     }
// }

// void Connection::adoptWriteBuffer(WriteBuffer* writeBuffer) {
//     if (writeBuffer_ != writeBuffer) {
//         if (writeBuffer_) {
//             delete writeBuffer_;
//         }
//         writeBuffer_ = writeBuffer;
//     }
// }
// void Connection::resetReadBuffer() {
//     if (readBuffer_) {
//         delete readBuffer_;
//         readBuffer_ = 0;
//     }
// }

// void Connection::resetWriteBuffer() {
//     if (writeBuffer_) {
//         delete writeBuffer_;
//         writeBuffer_ = 0;
//     }
// }
