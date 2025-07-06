#pragma once
#include <iostream>
#include <stdint.h>
#include <cstring>
#include <netinet/in.h>  // sockaddr_in, htons
#include <arpa/inet.h>   // inet_pton
#include <sys/socket.h>  // sockaddr_storage, sockaddr
#include <sys/types.h>


class SocketAddr {
    friend class ServerSocket;
public:
    static SocketAddr createIPv4(const std::string& hostName, uint16_t port);

    sockaddr* raw();
    socklen_t length() const;
    void setLength(socklen_t len);
    static void resolveByName(
        const std::string& hostName, 
        uint16_t port,
        sockaddr_in* addrIn);
    std::string SocketAddr::getAddress() const;
    uint16_t SocketAddr::getPort() const;
   private:
    SocketAddr() : length_(0) {};
    sockaddr_storage storage_;
    socklen_t length_;
};
