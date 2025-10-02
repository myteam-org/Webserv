#include "io/handler/CgiStdoutHandler.hpp"
#include "io/input/reader/fd.hpp"

CgiStdoutHandler::CgiStdoutHandler(Server* s) : srv_(s) {}

void CgiStdoutHandler::onEvent(const FdEntry& e, uint32_t m) {
    Connection* c = e.conn;
    if (!c || !c->isCgiActive()) 
        return;
    if (m & EPOLLERR) {
        LOG_ERROR("CgiStdoutHandler::onEvent: CGI epoll error");
        DispatchResult d1 = srv_->getDispatcher()->emitError(*c, http::kStatusBadGateway, "CGI stdout EPOLLERR");
        srv_->applyDispatchResult(*c, d1);
        srv_->applyDispatchResult(*c, DispatchResult::CgiAbort());
        return;
    }
    if (m & EPOLLIN) {
        io::FdReader r(c->getCgi()->getFdOut());
        char buf[8192];
        for (;;) {
            io::FdReader::ReadResult rr = r.read(buf, sizeof(buf));
            const size_t n = rr.unwrap();
            if (n == 0) {
                if (r.eof()) {
                    DispatchResult d1 = srv_->getDispatcher()->finalizeCgi(*c);
                    srv_->applyDispatchResult(*c, d1);
                    srv_->applyDispatchResult(*c, DispatchResult::CgiCloseOut());
                }
                break;
            }
            c->getCgi()->setRawOut(c->getCgi()->getRawOut().append(buf, buf + n));
        }
    }
    if (m & EPOLLHUP) {
        DispatchResult d1 = srv_->getDispatcher()->finalizeCgi(*c);
        srv_->applyDispatchResult(*c, d1);
        srv_->applyDispatchResult(*c, DispatchResult::CgiCloseOut());
    }
}
