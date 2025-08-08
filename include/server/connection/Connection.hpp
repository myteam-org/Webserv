#include "ConnectionSocket.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/write/buffer.hpp"
#include "IConnectionState.hpp"


class Connection {
private:
    ConnectionSocket connSock_;
    ReadBuffer *readBuffer_;
    WriteBuffer *writeBuffer_;
    IConnectionState *connState_;
    time_t lastRecv_;
    Connection(const Connection&);
    Connection& operator=(const Connection&);
    static const int kTimeoutThresholdSec = 60;

public:
    Connection (int fd, const ISocketAddr& peerAddr);
    ~Connection();
    const ConnectionSocket& getConnSock() const;
    ReadBuffer* getReadBuffer() const;
    void adoptReadBuffer(ReadBuffer* readBuffer);
    WriteBuffer* getWriteBuffer() const;
    void adoptWriteBuffer(WriteBuffer* writeBuffer);
    IConnectionState* getConnState() const;
    void setConnState(IConnectionState* connState);
    time_t getLastRecv() const;
    void setLastRecv(time_t lastRecv);
    bool isTimeout();
    void resetReadBuffer();
    void resetWriteBuffer();
};