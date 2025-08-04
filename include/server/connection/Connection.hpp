#include "ConnectionSocket.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/write/buffer.hpp"
#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "IConnectionState.hpp"


class Connection {
private:
   const  ConnectionSocket& connSock_;
    ReadBuffer *readBuffer_;
    WriteBuffer *writeBuffer_;
    IConnectionState *connState_;
    http::Request *httpRequest_;
    http::Response *httpResponse_;
    time_t lastRecv_;
    Connection(const Connection&);
    Connection& operator=(const Connection&);
    static const int kTimeoutThresholdSec = 60;

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
    void setHttpRequest(http::Request* httpRequest);
    http::Response* getHttpResponse() const;
    void setHttpResponse(http::Response* httpResponse);
    time_t getLastRecv() const;
    void setLastRecv(time_t lastRecv);
    bool isTimeout();
    void resetRequest();
    void resetResponse();
    void resetReadBuffer();
    void resetWriteBuffer();
};