#pragma once
#include "server/socket/ConnectionSocket.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/write/buffer.hpp"
#include "server/connection/state/IConnectionState.hpp"
 #include "http/request/read/reader.hpp"
#include <ctime>
#include <deque>
#include "http/request/request.hpp"
#include "action/cgi_action.hpp"
#include "action/cgi_context.hpp"

class Connection {
private:
    ConnectionSocket connSock_;
    ReadBuffer readBuffer_;
    WriteBuffer writeBuffer_;
    http::RequestReader requestReader_;
    Connection(const Connection&);
    Connection& operator=(const Connection&);
    std::deque<http::Request> pending_;
    bool frontDispatched_;
    bool closeAfterWrite_;
    bool peerHalfClosed_;
    time_t lastRecv_;
    PreparedCgi* preparedCgi_;
    CgiContext*  cgi_;


public:
    Connection(int fd, const ISocketAddr& peerAddr,
                       http::config::IConfigResolver& resolver);

    ~Connection();
    const ConnectionSocket& getConnSock() const;
    const ReadBuffer& getReadBuffer() const;
    const WriteBuffer& getWriteBuffer() const;
    ReadBuffer& getReadBuffer();
    WriteBuffer& getWriteBuffer();
    time_t getLastRecv() const;
    const http::RequestReader& getRequestReader() const;
    int getFd() const;
    void setLastRecv(time_t lastRecv);
    bool isTimeout() const;
    bool hasPending() const;
    void popFront();
    http::Request& front();
    void pushCreatedReq(http::Request req);
    http::RequestReader& getRequestReader();
    bool isPeerHalfClosed() const;
    void onPeerHalfClose();
    bool shouldCloseAfterWrite() const;
    void markCloseAfterWrite();
    bool isFrontDispatched() const;
    void markFrontDispatched();
    void resetFrontDispatched();
    void setPreparedCgi(const PreparedCgi& p);
    PreparedCgi* takePreparedCgi();
    void clearPreparedCgi();
    void setCgi(CgiContext* x);
    CgiContext* getCgi() const;
    bool isCgiActive() const;
    void clearCgi();
    static const std::time_t kTimeoutThresholdSec = 20;
};
