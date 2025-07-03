#pragma once

#include "ISocket.hpp"
#include <sys/socket.h>
#include "SocketAddr.hpp"

class ServerSocket : public ISocket {
public:
    ServerSocket(
        uint16_t port,
        int domain = AF_INET,
        int type = SOCK_STREAM,
        int protocol = kDefaultProtocol);
    ~ServerSocket();
    virtual int getRawFd() const;
    uint16_t getBindPort() const;
    std::string getBindAddress() const;
    void setBindPort(uint16_t port);
    void setBindAddress(std::string address);	
    types::Result<int, int> socket(
        int domain = AF_INET, 
        int type = SOCK_STREAM, 
        int protocol = kDefaultProtocol);
    types::Result<int, int> ServerSocket::bind(FileDescriptor fd, SocketAddr sockAddr);
    types::Result<int, int> listen(FileDescriptor &fd, );
    types::Result<int, int> accept(FileDescriptor &fd, );
    static const std::string kDefaultIp;

private:
    FileDescriptor fd_;
    std::string bindAddress_;
    uint16_t bindPort_;
	static const int kDefaultProtocol = 0;
};

void resolveByName(const std::string& hostname, uint16_t port);