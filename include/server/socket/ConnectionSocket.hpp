#pragma once

#include "ISocket.hpp"
#include "SocketAddr.hpp"
#include "ConnectionSocket.hpp"
#include <sstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>

class ConnectionSocket : public ISocket {
public:
    ConnectionSocket(int fd, const ISocketAddr& peerAddr);
    ConnectionSocket(int fd, const SocketAddr& peerAddr); 
    ~ConnectionSocket();
    virtual int getRawFd() const;
    uint16_t getPeerPort() const;
    std::string getPeerAddress() const;
    std::string getClientInfo() const;

private:
    FileDescriptor fd_;
    std::string peerAddress_;
    uint16_t peerPort_;
    static const int kInvalidFd = -1;
    ConnectionSocket(const ConnectionSocket&);
};
