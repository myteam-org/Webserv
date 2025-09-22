#pragma once
#include "io/handler/IFdHandler.hpp"
#include "server/Server.hpp"

class ClientHandler : public IFdHandler {
public:
    explicit ClientHandler(Server* s);
    void onEvent(const FdEntry& e, uint32_t m);
private: 
    Server* srv_;
    void onReadable(Connection& c);
    void onWritable(Connection& c);
    void handleHangup(Connection& c);
    void handlePeerHalfClose(Connection& c);
    void maybeDispatch(Connection& c);
};