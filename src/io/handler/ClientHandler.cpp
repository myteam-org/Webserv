#pragma once
#include "io/handler/IFdHandler.hpp"
#include "server/Server.hpp"
#include "http/request/read/reader.hpp"
#include "io/input/writer/writer.hpp"

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
        maybeDispatch(*c);
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
        const http::RequestReader::ReadRequestResult readRes = c.getRequestReader().readRequest(c.getReadBuffer());
        if (readRes.isErr()) {
            failAndClose(c, readRes.unwrapErr()); // 400等へ
            return;
        }
        types::Option<http::Request> reqOpt = readRes.unwrap();
        // request 完成判断
        if (!reqOpt.isNone()) {
            const http::Request& req = reqOpt.unwrap();
            c.pushCreatedReq(req);
            continue; // さらに取れるだけ回す
        }
        // NeedMore
        break;
    }
}

void ClientHandler::maybeDispatch(Connection& c) {
    // 先頭がまだ dispatch されておらず、書きかけがない時だけ
    if (!c.hasPending()) {
        return;
    }
    if (c.isFrontDispatched()) {
        return;
    }
    if (!c.getWriteBuffer().isEmpty()) {
        return;
    }
    DispatchResult dispatchResult = srv_->getDispatcher()->step(c);  // ← ここで markFrontDispatched() も行う（後述）
    srv_->applyDispatchResult(c, dispatchResult);
}

void ClientHandler::onWritable(Connection& c) {
    WriteBuffer &writeBuffer = c.getWriteBuffer();
    for (;;) {
        types::Result<std::size_t, error::AppError>  writeRes = writeBuffer.flush();
        if (writeRes.isErr()) { 
            handleWriteError(c, errno);
            return; 
        }
        if (writeRes.unwrap() == 0) {
            break;
        }
    }
    if (writeBuffer.isEmpty()) {
        if (c.hasPending()) {
            c.popFront(); // popでresetFrontDispatched()
        }
        if (c.shouldCloseAfterWrite() || c.isPeerHalfClosed()) {
            srv_->applyDispatchResult(c, DispatchResult::kClose);
            return;
        }
        if (c.hasPending()) {
            maybeDispatch(c);  // パイプライン前進
        } else {
            srv_->armInOnly(c.getFd());
        }
    }
}

void ClientHandler::handleHangup(Connection& c) {
    srv_->applyDispatchResult(c, DispatchResult::kClose);
}

void ClientHandler::handlePeerHalfClose(Connection& c) {
    c.onPeerHalfClose();
}