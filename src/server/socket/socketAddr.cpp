#include "SocketAddr.hpp"

SocketAddr SocketAddr::createIPv4(const std::string& ip, uint16_t port) {
    SocketAddr sockAddr;
    sockaddr_in addrIn;
    std::memset(&addrIn, 0, sizeof(sockaddr_in));

    if (inet_pton(AF_INET, ip.c_str(), &addrIn.sin_addr) == 1) {
        addrIn.sin_family = AF_INET;
        addrIn.sin_port = htons(port);
    } else {
        resolveByName(ip, port);
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

void resolveByName(const std::string& hostname, uint16_t port) {
    
}