#pragma once
#include <iostream>
#include <stdint.h>
#include <cstring>
#include <netinet/in.h>  // sockaddr_in, htons
#include <arpa/inet.h>   // inet_pton
#include <sys/socket.h>  // sockaddr_storage, sockaddr
#include <sys/types.h>
#include "ISocketAddr.hpp"


class SocketAddr : public ISocketAddr {
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
    virtual std::string getAddress() const;
    virtual uint16_t getPort() const;
    static const int kFirstOctetShift = 24;
    static const int kSecondOctetShift = 16;
    static const int kThirdOctetShift = 8;
    static const int kFourthOctetShift = 0;
    static const uint32_t kOctetMask = 0xFF;
private:
    SocketAddr() : length_(0) {};
    sockaddr_storage storage_;
    socklen_t length_;
};
