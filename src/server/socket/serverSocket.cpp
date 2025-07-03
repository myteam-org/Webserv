#include "ServerSocket.hpp"

ServerSocket::ServerSocket(uint16_t port, int domain, int type, int protocol) {    
    const types::Result<int, int> socketFd = socket(domain, type, protocol);
    if (socketFd.isErr()) {
        throw std::runtime_error("socket creation failed");
    }
    //config から ipaddress を受け取る場合変更する必要あり。
    SocketAddr sockAddr = SocketAddr::createIPv4(kDefaultIp, port);
    bind(FileDescriptor(socketFd.unwrap()), sockAddr);

}

ServerSocket::~ServerSocket(){

}

int ServerSocket::getRawFd() const {

}

uint16_t ServerSocket::getBindPort() const {
    return bindPort_;
}

std::string ServerSocket::getBindAddress() const {
    return bindAddress_;
}

void ServerSocket::setBindAddress(std::string address) {
    bindAddress_ = address;
}

void ServerSocket::setBindPort(uint16_t port) {
    bindPort_ = port;
}

types::Result<int, int> ServerSocket::socket(int domain, int type, int protocol) {

}

types::Result<int, int> ServerSocket::bind(FileDescriptor fd, SocketAddr sockAddr) {
    ::bind(fd.getFd().unwrap(), sockAddr.raw(), sockAddr.length());
}


