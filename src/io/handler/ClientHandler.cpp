#pragma once
#include "io/handler/IFdHandler.hpp"
#include "server/Server.hpp"

explicit ClientHandler::ClientHandler(Server* s) : srv_(s) {};

void ClientHandler::onEvent(const FdEntry& e, uint32_t m){
    Connection* c = e.conn;
    if (!c) {
        return;
    }
    if (m & (EPOLLERR | EPOLLHUP)) {
       handleHangup(*c);
       return;
    }
    if (m & EPOLLRDHUP) { 
         handlePeerHalfClose(*c);
    }
    if (m & EPOLLIN) { 
        onReadable(*c);
        if (c->hasPending()) {
            DispatchResult dr = srv_->dispatcher().step(*c);
            srv_->applyDispatchResult(*c, dr);
        }
    }
    if (m & EPOLLOUT) {
        onWritable(*c);
    }
};


void ClientHandler::onReadable(Connection& c) {
    // 1) 読む（WouldBlock まで）
    for (;;) {
        ReadBuffer::LoadResult lr = c.getReadBuffer().load();
        if (lr.isErr()) {
            // EPOLLERR/EPOLLRDHUP と整合させてクローズへ
            handleReadError(c, lr.unwrapErr());
            return;
        }
        if (lr.unwrap() == 0) break; // これ以上は読めない
        c.setLastRecv(std::time(0));
    }

    // 2) 進める（取れるだけ）
    for (;;) {
        http::StepResult step = c.getRequestReader().tryParseOne(c.getReadBuffer());
        if (step.isErr()) {
            failAndClose(c, step.unwrapErr()); // 400等へ
            return;
        }
        const http::ParseProgress& p = step.unwrap();
        if (p.code == http::ParseProgress::kCompleted) {
            // 1件完成
            const http::Request& req = p.req.unwrap();
            c.pendingPush(req);
            c.getRequestReader().resetForNext(); // 次のリクエスト準備（パイプライン可）
            continue; // さらに取れるだけ回す
        }
        // NeedMore
        break;
    }

    // 必要ならここで dispatcher 呼ぶ（front が QUEUED なら、など）
    maybeDispatch(c);
}


void ClientHandler::onWritable(Connection& c) {
    // c.writeBuffer().flush();
    // flush完了で popFront(), close 判定
}

void ClientHandler::handleHangup(Connection& c) {
    srv_->applyDispatchResult(c, DispatchResult::kClose);
}

void ClientHandler::handlePeerHalfClose(Connection& c) {
    c.onPeerHalfClose(); // これは Connection 内の状態フラグ更新だけ
}