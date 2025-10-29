#include "server/dispatcher/RequestDispatcher.hpp"
#include "action/cgi_action.hpp"
#include "utils/logger.hpp"
#include "http/response/response.hpp"

RequestDispatcher::RequestDispatcher(EndpointResolver& resolver) : resovler_(resolver) {}

DispatchResult RequestDispatcher::step(Connection& c) {
    if (!c.hasPending()) {
        return DispatchResult::kNone;
    }
    http::Request& req = c.front();
    DispatchResult dispatchResult = dispatchNext(c, req);
    if (dispatchResult.isArmOut()) {
        c.markFrontDispatched();
    }
    return dispatchResult;
}

DispatchResult RequestDispatcher::dispatchNext(Connection& c, http::Request& req) {
    VirtualServer *vserver = resovler_.resolveByFd(c.getFd());
    if (!vserver) { 
        LOG_WARN("RequestDispatcher::dispatchNext: Cannot find any virtual server from filedescriptor");
        http::ResponseBuilder rb;
        rb.status(http::kStatusBadRequest).text("Bad Request", http::kStatusBadRequest);
        enqueueResponse(c, rb.build());
        // close方針にするなら:
        c.markCloseAfterWrite();
        return DispatchResult::ArmOut();
    }
    Either<IAction*, http::Response> responseRes = vserver->getRouter().serve(req);
    if (responseRes.isRight()) {
        http::Response resp = responseRes.unwrapRight();
        // c.markCloseAfterWrite();
        enqueueResponse(c, resp);        // WriteBufferへ
        return DispatchResult::ArmOut();
    }
    IAction* action = responseRes.unwrapLeft();
    CgiActionPrepared* cgi = dynamic_cast<CgiActionPrepared*>(action);
    if (cgi) {
        LOG_DEBUG("RequestDispatcher::dispatchNext: CGI preparetion is done.");
        c.setPreparedCgi(cgi->payload());
        delete action;
        return DispatchResult::StartCgi(CgiFds());
    }
    LOG_WARN("RequestDispatcher::dispatchNext: Doesn't invoke any handler.");
    http::ResponseBuilder rb;
    rb.status(http::kStatusNotImplemented).text("Not Implemented", http::kStatusNotImplemented);
    enqueueResponse(c, rb.build());
    return DispatchResult::ArmOut();
}

DispatchResult RequestDispatcher::finalizeCgi(Connection& c) {
    if (!c.isCgiActive()) {
        return DispatchResult::kNone;
    }
    CgiContext* ctx = c.getCgi();
    http::Response resp = ctx->invokeParser();
    enqueueResponse(c, resp);
    return DispatchResult::ArmOut();
}

void RequestDispatcher::enqueueResponse(Connection& c, const http::Response& resp) {
    const std::string raw = const_cast<http::Response&>(resp).toString();
    c.getWriteBuffer().append(raw);
}

DispatchResult RequestDispatcher::emitError(Connection& c,
                                            http::HttpStatusCode status,
                                            const std::string& plain) {
    VirtualServer* vs = resovler_.resolveByFd(c.getFd());
    http::Response resp = buildErrorResponse(vs, status, plain);
    c.markCloseAfterWrite();
    enqueueResponse(c, resp);
    return DispatchResult::ArmOut();
}

http::Response RequestDispatcher::buildErrorResponse(
        VirtualServer* vs, http::HttpStatusCode status, const std::string& plain) {
    // TODO: vs に error_page 設定があればファイルを読み込み、
    //             body/Content-Type を差し替える処理をここに追加
    (void)vs;
    http::ResponseBuilder rb;
    rb.status(status).header("Content-Type", "text/plain");
    rb.text(plain, status);
    return rb.build();
}
