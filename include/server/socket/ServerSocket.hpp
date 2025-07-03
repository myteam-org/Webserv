#pragma once

#include "ISocket.hpp"

class ServerSocket : public ISocket {
public:
    ServerSocket(
        uint16_t port,
        int domain = AF_INET,
        int type = SOCK_STREAM,
        int protocol = 0);
    ~ServerSocket();
    virtual int getRawFd() const;
    uint16_t getBindPort() const;
    std::string getBindAddress() const;
    void setBindPort(uint16_t port);
    std::string setBindAddress(std::string address);	
    types::Result<int, int> socket(int domain, int type, int protocol);
    types::Result<int, int> bind(uint16_t port);
    types::Result<int, int> listen(FileDescriptor &fd, );
    types::Result<int, int> accept(FileDescriptor &fd, );

private:
    FileDescriptor fd_;
    std::string bindAddress_;
    uint16_t bindPort_;
}

