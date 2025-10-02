#include "io/handler/CgiStdinHandler.hpp"
#include "io/input/writer/fd.hpp" 
#include "server/Server.hpp"
#include "server/dispatcher/RequestDispatcher.hpp"

CgiStdinHandler::CgiStdinHandler(Server* s) : srv_(s) {}

void CgiStdinHandler::onEvent(const FdEntry& e, uint32_t m) {
    Connection* c = e.conn;
    if (!c || !c->isCgiActive()) return;
    CgiContext* ctx = c->getCgi();
    if (m & EPOLLERR) {
        DispatchResult d1 = srv_->getDispatcher()->emitError(
            *c, http::kStatusBadGateway, "CGI stdin EPOLLERR");
        srv_->applyDispatchResult(*c, d1);
        srv_->applyDispatchResult(*c, DispatchResult::CgiAbort());
        return;
    }
    if ((m & EPOLLOUT) && ctx->getFdIn() >= 0 && ctx->getStdinBody()) {
        io::FdWriter w(ctx->getFdIn());
        const std::vector<char>& body = *ctx->getStdinBody();
        while (ctx->getWritten() < body.size()) {
            io::FdWriter::WriteResult wr = w.write(&body[ctx->getWritten()], body.size() - ctx->getWritten());
            const std::size_t n = wr.unwrap();
            if (n == 0)
                break;
            ctx->setWritten(ctx->getWritten() + n);
        }
        if (ctx->getWritten() >= body.size()) {
            srv_->applyDispatchResult(*c, DispatchResult::CgiCloseIn());
        }
    }
    if (m & EPOLLHUP) {
        srv_->applyDispatchResult(*c, DispatchResult::CgiCloseIn());
    }
}
