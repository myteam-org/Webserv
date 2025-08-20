#pragma once

#include "ISocket.hpp"
#include "SocketAddr.hpp"
#include <sstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include "io/input/reader/reader.hpp"
#include "io/input/writer/writer.hpp"

class ConnectionSocket : public ISocket, public io::IReader, public io::IWriter {
public:
    ConnectionSocket(int fd, const ISocketAddr& peerAddr);
    ~ConnectionSocket();
    virtual int getRawFd() const;
    uint16_t getPeerPort() const;
    std::string getPeerAddress() const;
    std::string getClientInfo() const;
    ReadResult read(char* buf, std::size_t n);
    WriteResult write(const char* buf, std::size_t n);
    bool eof();

private:
    FileDescriptor fd_;
    std::string peerAddress_;
    uint16_t peerPort_;
    static const int kInvalidFd = -1;
    ConnectionSocket(const ConnectionSocket&);
    bool eof_;
};
