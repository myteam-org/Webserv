#include "server/dispatcher/RequestDispatcher.hpp"

//直列化した文字列を積む
// void pushRawToWriteBuffer(Connection& c, const std::string& bytes) {
//         c.getWriteBuffer().append(bytes);
// }

DispatchResult RequestDispatcher::step(Connection& c) {
    if (!c.hasPending()) {
        return DispatchResult::kNone;
    }
    http::Request& req = c.front(); // pending_ の先頭を参照
    DispatchResult dispatchResult = dispatchNext(c, req);
    if (dispatchResult.isArmOut()) {
        c.markFrontDispatched();
    }
    return dispatchResult;
}

DispatchResult RequestDispatcher::dispatchNext(Connection& c, http::Request& req) {
    VirtualServer *vserver = resovler_.resolveByFd(c.getFd());
    if (!vserver) { 
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
        // 接続方針を Connection に伝える
        if (shouldClose(req)) {
            c.markCloseAfterWrite();
        }
        enqueueResponse(c, resp);        // WriteBufferへ
        return DispatchResult::ArmOut();
    }
    //To Do : cgi 処理など
    http::ResponseBuilder rb;
    rb.status(http::kStatusNotImplemented).text("Not Implemented", http::kStatusNotImplemented);
    enqueueResponse(c, rb.build());
    return DispatchResult::ArmOut();
}

DispatchResult RequestDispatcher::startCgi(Connection& /*c*/, const std::string& /*scriptPath*/) {
    // ここでは本当に fork/exec しない。Server 側配線だけ先に動かすためのモック。
    CgiFds fds; fds.stdin_fd = -1; fds.stdout_fd = -1;
    return DispatchResult::StartCgi(fds);
}

DispatchResult RequestDispatcher::onCgiStdout(Connection& c) {
    // 本来は CGI 出力からヘッダを切り出して Response へ。モックでは 200 固定で返す。
    http::ResponseBuilder rb;
    rb.status(http::kStatusOk)
      .header("Content-Type", "text/plain")
      .text("cgi output", http::kStatusOk);
    enqueueResponse(c, rb.build());
    return DispatchResult::ArmOut();
}

DispatchResult RequestDispatcher::onCgiStdin(Connection& /*c*/) {
    // 本来はリクエストボディを stdin へ。モックでは何もしない
    return DispatchResult::kNone;
}

void RequestDispatcher::enqueueResponse(Connection& c, const http::Response& resp) {
    const std::string raw = const_cast<http::Response&>(resp).toString();
    c.getWriteBuffer().append(raw);
}

// bool RequestDispatcher::wantsCgi(Connection& c) const {
//     const std::string uri = getRequestTarget(c);
//     // 超簡易：.cgi とか .py なら CGI とみなす
//     const std::string::size_type dot = uri.rfind('.');
//     if (dot == std::string::npos) return false;
//     const std::string ext = uri.substr(dot);
//     return (ext == ".cgi" || ext == ".py" || ext == ".php");
// }
