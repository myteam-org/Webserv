#include "ConnectionSocket.hpp"

ConnectionSocket::ConnectionSocket(int fd, const ISocketAddr& peerAddr) {
    fd_ = FileDescriptor(fd);
    peerAddress_ = peerAddr.getAddress();
    peerPort_ = peerAddr.getPort();
}

ConnectionSocket::ConnectionSocket(int fd, const SocketAddr& peerAddr) {
    fd_ = FileDescriptor(fd);
    peerAddress_ = peerAddr.getAddress();
    peerPort_ = peerAddr.getPort();
}

ConnectionSocket::~ConnectionSocket() {}

int ConnectionSocket::getRawFd() const {
    return fd_.getFd().unwrapOr(kInvalidFd);
}

uint16_t ConnectionSocket::getPeerPort() const {
    return peerPort_;
}

std::string ConnectionSocket::getPeerAddress() const {
    return peerAddress_;
} 

std::string ConnectionSocket::getClientInfo() const {
    std::ostringstream oss;
    oss << peerAddress_ << ":" << peerPort_;
    return oss.str();
}

const int ConnectionSocket::kInvalidFd;
