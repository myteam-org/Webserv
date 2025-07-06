#include "SocketAddr.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sstream> 

SocketAddr SocketAddr::createIPv4(const std::string& hostName, uint16_t port) {
    SocketAddr sockAddr;
    sockaddr_in addrIn;
    std::memset(&addrIn, 0, sizeof(sockaddr_in));

    if (inet_pton(AF_INET, hostName.c_str(), &addrIn.sin_addr) == 1) {
        addrIn.sin_family = AF_INET;
        addrIn.sin_port = htons(port);
    } else {
        resolveByName(hostName, port, &addrIn);
    }
    std::memset(&sockAddr.storage_, 0, sizeof(sockAddr.storage_));
    std::memcpy(&sockAddr.storage_, &addrIn, sizeof(addrIn));
    sockAddr.length_ = sizeof(addrIn);
    return sockAddr;
}

sockaddr* SocketAddr::raw(){
    return reinterpret_cast<sockaddr*>(&storage_);
}

socklen_t SocketAddr::length() const {
    return length_;
}

void SocketAddr::setLength(socklen_t len) {
    length_ = len;
}

void SocketAddr::resolveByName(
    const std::string& hostName, 
    uint16_t port, 
    sockaddr_in *addrIn
    ) {
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* res;
    int err = getaddrinfo(hostName.c_str(), NULL, &hints, &res);
    if (err != 0) {
        throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(err)));
    }
    sockaddr_in* resolvedAddr = reinterpret_cast<sockaddr_in*>(res->ai_addr);
    std::memcpy(addrIn, resolvedAddr, sizeof(sockaddr_in));
    addrIn = reinterpret_cast<sockaddr_in*>(res->ai_addr);
    addrIn->sin_port = htons(port);
}

std::string SocketAddr::getAddress() const {
    const sockaddr_in* addr = reinterpret_cast<const sockaddr_in*>(&storage_);
    uint32_t ip = ntohl(addr->sin_addr.s_addr);
    unsigned char a = (ip >> 24) & 0xFF;
    unsigned char b = (ip >> 16) & 0xFF;
    unsigned char c = (ip >> 8) & 0xFF;
    unsigned char d = ip & 0xFF;
    std::ostringstream oss;
    oss << static_cast<int>(a) << "."
        << static_cast<int>(b) << "."
        << static_cast<int>(c) << "."
        << static_cast<int>(d);
    return oss.str();
}

uint16_t SocketAddr::getPort() const {
    const sockaddr_in* addr = reinterpret_cast<const sockaddr_in*>(&storage_);
    return ntohs(addr->sin_port);
}