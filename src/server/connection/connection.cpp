#include "server/connection/Connection.hpp"
#include <ctime>
#include "io/input/reader/fd.hpp"

Connection::Connection(int fd, const ISocketAddr& peerAddr,
                       http::config::IConfigResolver& resolver)
    : connSock_(fd, peerAddr)
    , readBuffer_(connSock_)
    , writeBuffer_(connSock_)
    , requestReader_(resolver)
    , frontDispatched_(false)
    , closeAfterWrite_(false)
    , peerHalfClosed_(false)
    , lastRecv_(std::time(0))
    , cgi_(0)
    , preparedCgi_(0) {}

Connection::~Connection() {
    if (cgi_ != NULL) {
        delete cgi_;
    }
}

const ConnectionSocket& Connection::getConnSock() const {
    return connSock_;
}

ReadBuffer& Connection::getReadBuffer() {
    return readBuffer_;
}
const ReadBuffer& Connection::getReadBuffer() const {
    return readBuffer_;
}

WriteBuffer& Connection::getWriteBuffer() {
    return writeBuffer_;
}

const WriteBuffer& Connection::getWriteBuffer() const {
    return writeBuffer_;
}

time_t Connection::getLastRecv() const {
    return lastRecv_;
}

void Connection::setLastRecv(time_t lastRecvVal) {
    lastRecv_ = lastRecvVal;
}

bool Connection::isTimeout() const {
    time_t now = std::time(0);
    return (now - lastRecv_) > kTimeoutThresholdSec;
}

bool Connection::hasPending() const { 
    return !pending_.empty();
}
http::Request& Connection::front(){ 
    return pending_.front();
}

void Connection::popFront() { 
    pending_.pop_front();
    resetFrontDispatched();
}

void Connection::pushCreatedReq(http::Request req) { 
    pending_.push_back(req);
}

bool Connection::isPeerHalfClosed() const {
    return peerHalfClosed_;
}

void Connection::onPeerHalfClose() {
    peerHalfClosed_ = true;
}

bool Connection::shouldCloseAfterWrite() const {
    return closeAfterWrite_;
}

void Connection::markCloseAfterWrite() {
    closeAfterWrite_ = true;
}

bool Connection::isFrontDispatched() const {
    return frontDispatched_;
}

void Connection::markFrontDispatched() {
    frontDispatched_ = true;
}

void Connection::resetFrontDispatched() {
    frontDispatched_ = false;
}

http::RequestReader& Connection::getRequestReader() {
    return requestReader_;
}

const http::RequestReader& Connection::getRequestReader() const {
    return requestReader_;
}

int Connection::getFd() const {
    return connSock_.getRawFd();
}

void Connection::setPreparedCgi(const PreparedCgi& p) { 
    clearPreparedCgi();
    preparedCgi_ = new PreparedCgi(p);
}

PreparedCgi* Connection::takePreparedCgi() {
    PreparedCgi* t = preparedCgi_;
    preparedCgi_ = 0; 
    return t;
}

void Connection::clearPreparedCgi() {
    if(preparedCgi_){ 
        delete preparedCgi_;
        preparedCgi_=0;
    }
}

void Connection::setCgi(CgiContext* x) {
    cgi_ = x;
}

CgiContext* Connection::getCgi() const {
    return cgi_;
}

bool Connection::isCgiActive() const {
    return cgi_ != 0;
}

void Connection::clearCgi() {
    if (cgi_) {
        delete cgi_;
        cgi_ = 0;
    }
}