#pragma once

#include "ISocket.hpp"
class ConnectionSocket : public ISocket {
public:
    ConnectionSocket();
    ~ConnectionSocket();
    virtual int getRawFd() const;
    uint16_t getPeerPort() const;
    std::string getPeerAddress() const;

private:
    FileDescriptor fd_;
    std::string peerAddress;
    uint16_t peerPort_;
};
