#include "ServerSocket.hpp"
#include "utils/types/result.hpp"
#include <cerrno>
#include <iostream>
#include <cstring>

ServerSocket::ServerSocket(
    uint16_t port, 
    const std::string &hostName,
    int domain, 
    int type, 
    int protocol
    ) {
    const types::Result<int, int> socketFd = socket(domain, type, protocol);
    if (socketFd.isErr()) {
        throw std::runtime_error("socket creation failed");
    }
    SocketAddr sockAddr = SocketAddr::createIPv4(hostName, port);
    setFd(socketFd.unwrap());
    const types::Result<int, int> bindRet = bind(sockAddr);
    if (bindRet.isErr()) {
        throw std::runtime_error("bind failed");
    }
    bindPort_ = port;
    bindAddress_ = hostName;
    const types::Result<int, int> listenRet = listen(SOMAXCONN);
    if (listenRet.isErr()) {
        throw std::runtime_error("listen failed");
    }
}

ServerSocket::~ServerSocket() {

}

int ServerSocket::getRawFd() const {
    return fd_.getFd().unwrapOr(kInvalidResult); 
}

uint16_t ServerSocket::getBindPort() const {
    return bindPort_;
}

std::string ServerSocket::getBindAddress() const {
    return bindAddress_;
}

void ServerSocket::setBindAddress(std::string& address) {
    bindAddress_ = address;
}

void ServerSocket::setBindPort(uint16_t port) {
    bindPort_ = port;
}

void ServerSocket::setFd(int fd) {
    fd_.setFd(fd);
}

types::Result<int, int> ServerSocket::socket(
    int domain, 
    int type, 
    int protocol
    ) {
    const int res = ::socket(domain, type, protocol);
    if (res == kInvalidResult) {
        return ERR(errno);
    }
    return OK(res);
}

types::Result<int, int> ServerSocket::bind(SocketAddr &sockAddr) const{
    sockaddr* addr = sockAddr.raw();
    const socklen_t len = sockAddr.length();
    const int res = ::bind(getRawFd(), addr, len);
    if (res == kInvalidResult) {
        return ERR(errno);
    }
    return OK(res);
}

types::Result<int, int> ServerSocket::listen(int backlog) const {
    const int res = ::listen(getRawFd(), backlog);
    if (res == kInvalidResult) {
        return ERR(errno);
    }
    return OK(res);
}

ServerSocket::ConnectionResult ServerSocket::accept() const{
    SocketAddr clientAddr;
    socklen_t addrLen = sizeof(sockaddr_storage);
    const int res = ::accept(
        getRawFd(), 
        clientAddr.raw(), 
        &addrLen
        );
    if (res == kInvalidResult) {
        return ERR(errno);
    }
    clientAddr.setLength(addrLen);  
    return OK(new ConnectionSocket(res, clientAddr));
}

const int ServerSocket::kDefaultProtocol = 0;
const int ServerSocket::kInvalidResult = -1;