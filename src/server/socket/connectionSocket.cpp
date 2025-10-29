#include "server/socket/ConnectionSocket.hpp"

ConnectionSocket::ConnectionSocket(int fd, const ISocketAddr& peerAddr) 
    : fd_(fd),
    peerAddress_ (peerAddr.getAddress()),
    peerPort_(peerAddr.getPort()),
    read_eof_(false),
    write_eof_(false) {}

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

io::IReader::ReadResult ConnectionSocket::read(char* buf, std::size_t n) {
    ssize_t r = ::recv(getRawFd(), buf, n, 0);
    if (r > 0) {
        return types::ok<std::size_t>(static_cast<size_t>(r));
    }
    if (r == 0) {
        read_eof_ = true;
        return types::ok<std::size_t>(0);
    }
    return types::ok<std::size_t>(0);
}

io::IWriter::WriteResult ConnectionSocket::write(const char* buf, std::size_t n) {
    ssize_t w = ::send(getRawFd(), buf, n, 0);
    if (w >= 0) {
        return types::ok<std::size_t>(static_cast<size_t>(w));
    }
    return types::ok<std::size_t>(0);
}

bool ConnectionSocket::eof() {
    return read_eof_;
}

bool ConnectionSocket::writeEof() {
    return write_eof_;
}

const int ConnectionSocket::kInvalidFd;
