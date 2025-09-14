#include "io/handler/Listenhandler.hpp"

ListenerHandler::ListenerHandler(Server* s) : srv_(s) {};

void ListenerHandler::onEvent(const FdEntry& e, uint32_t m) {
    if (m & (EPOLLERR|EPOLLHUP)) {
        return;
    }
    srv_->acceptLoop(e.fd);
}