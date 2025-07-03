#pragma once
#include <iostream>
#include <cstdint>
#include <bits/socket.h>
#include <cstring>
#include <netinet/in.h>  // sockaddr_in, htons
#include <arpa/inet.h>   // inet_pton
#include <sys/socket.h>  // sockaddr_storage, sockaddr
#include <cstdint>

class SocketAddr {
public:
    static SocketAddr createIPv4(const std::string& ip, uint16_t port);

    sockaddr* raw();
    socklen_t length() const;

private:
    SocketAddr() : length_(0) {};
    sockaddr_storage storage_;
    socklen_t length_;
};
