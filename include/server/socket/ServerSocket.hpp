#pragma once

#include "ISocket.hpp"
#include <sys/socket.h>
#include "SocketAddr.hpp"
#include "ConnectionSocket.hpp"

class ServerSocket : public ISocket {
public:
    ServerSocket(
        uint16_t port,
        int domain = AF_INET,
        int type = SOCK_STREAM,
        int protocol = kDefaultProtocol,
        const std::string &hostName);
    ~ServerSocket();
    virtual int getRawFd() const;
    void setFd(FileDescriptor fd);
    uint16_t getBindPort() const;
    std::string getBindAddress() const;
    void setBindPort(uint16_t port);
    void setBindAddress(std::string address);	
    types::Result<int, int> socket(
        int domain = AF_INET, 
        int type = SOCK_STREAM, 
        int protocol = kDefaultProtocol);
    types::Result<int, int> ServerSocket::bind(SocketAddr &sockAddr);
    types::Result<int, int> listen(int backlog);
    typedef types::Result<ConnectionSocket, int> ConnectionResult;
    ConnectionResult accept();
    static const std::string kDefaultIp;

private:
    FileDescriptor fd_;
    std::string bindAddress_;
    uint16_t bindPort_;
    static const int kDefaultProtocol = 0;
    static const int kInvalidResult = -1;
    void resolveByName(
        const std::string& hostName, 
        uint16_t port, 
        sockaddr_in *addrIn
        );
};
