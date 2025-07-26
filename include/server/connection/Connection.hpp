#include "ConnectionSocket.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/write/buffer.hpp"
#include "http/request/request.hpp"
#include "IConnectionState.hpp"


class Connection {
private:
   const  ConnectionSocket& connSock_;
    ReadBuffer *readBuffer_;
    WriteBuffer *writeBuffer_;
    IConnectionState *connState_;
    http::Request *httpRequest_;
    // http::Response *httpResponse;
    time_t lastRecv;
    Connection(const Connection&);
    Connection& operator=(const Connection&);

public:
    Connection (const ConnectionSocket& connSock);
    ~Connection();
    const ConnectionSocket& getConnSock() const;
    ReadBuffer* getReadBuffer() const;
    void setReadBuffer(ReadBuffer* readBuffer);
    WriteBuffer* getWriteBuffer() const;
    void setWriteBuffer(WriteBuffer* writeBuffer);
    IConnectionState* getConnState() const;
    void setConnState(IConnectionState* connState);
    http::Request* getHttpRequest() const;
    void setHttpRequest(http::Request* httpRequest) ;
    time_t getLastRecv() const;
    void setLastRecv(time_t lastRecv);
    bool isTimeout();
};