#include "io/handler/IFdHandler.hpp"
#include "server/Server.hpp"
#include "http/request/read/reader.hpp"
#include "io/input/writer/writer.hpp"

ClientHandler::ClientHandler(Server* s) : srv_(s) {}

void ClientHandler::onEvent(const FdEntry& e, uint32_t m){
    Connection* c = e.conn;
    if (!c) {
        return;
    }
    // 致命エラー
        if (m & EPOLLERR) {
        handleHangup(*c); //Connection close
        return;
    }
    // HUP かつ IN が有効
    bool hup = (m & EPOLLHUP);
    if ((m & EPOLLIN)) {
        onReadable(*c);
        // onReadable 内で failAndClose 済みならここで終了
        // （必要なら Connection に「closed」ビットを持たせて判定）
        maybeDispatch(*c);
    }
    // 受信側の半閉塞（送信は継続可能）
    if (m & EPOLLRDHUP) {
        handlePeerHalfClose(*c);
    }
    // 上部で IN を処理した後 HUP 処理
    if (hup) {
        handleHangup(*c);
        return;
    }
    if (m & EPOLLOUT) {
        onWritable(*c);
    }
}

void ClientHandler::onReadable(Connection& c) {
    // 1) 読む（WouldBlock まで）
    for (;;) {
        ReadBuffer::LoadResult lr = c.getReadBuffer().load();
        if (lr.isErr()) {
            // EPOLLERR/EPOLLRDHUP と整合させてクローズへ
            handleReadError(c, lr.unwrapErr());
            return;
        }
        if (lr.unwrap() == 0) {
            break; // これ以上は読めない
        }
        c.setLastRecv(std::time(0));
    }
    // 2) 進める（取れるだけ）
    for (;;) {
        const http::RequestReader::ReadRequestResult readRes = c.getRequestReader().readRequest(c.getReadBuffer());
        if (readRes.isErr()) {
            failAndClose(c, readRes.unwrapErr()); //400
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
    DispatchResult dispatchResult = srv_->getDispatcher()->step(c);
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
            c.popFront();
        }
        if (c.shouldCloseAfterWrite() || c.isPeerHalfClosed()) {
            srv_->applyDispatchResult(c, DispatchResult::Close());
            return;
        }
        if (c.hasPending()) {
            maybeDispatch(c);
        } else {
            srv_->armInOnly(c.getFd());
        }
    }
}

void ClientHandler::handleHangup(Connection& c) {
    srv_->applyDispatchResult(c, DispatchResult::Close());
}

void ClientHandler::handlePeerHalfClose(Connection& c) {
    c.onPeerHalfClose();
    srv_->armOutOnly(c.getFd());
}

void ClientHandler::failAndClose(Connection& c, const error::AppError& err) {
    const http::HttpStatusCode status = mapParseErrorToHttpStatus(err);
    DispatchResult dr = srv_->getDispatcher()->emitError(c, status, "Bad request");
    srv_->applyDispatchResult(c, dr);
}

void ClientHandler::handleWriteError(Connection& c, int sys_errno) {
    // TODO: ログ。必要なら SO_ERROR の吸い出しや errno→文字列化
    // if (isFatalIoErrno(sys_errno)) { ... }

    // 書き I/O エラーは即クローズ
    (void)sys_errno; // 未使用警告回避（ログを入れるなら不要）
    srv_->applyDispatchResult(c, DispatchResult::Close());
}

void ClientHandler::handleReadError(Connection& c, const error::AppError& err) {
    // TODO: ここで err から errno 相当のコードやメッセージを取得できるならログる
    // 例: int e = err.to_errno(); log("read error: %d", e);

    // 読み I/O エラーは即クローズでよい（レスポンスは返さない）
    srv_->applyDispatchResult(c, DispatchResult::Close());
}

// パース／前段バリデーションのエラーを HTTP ステータスへ
http::HttpStatusCode ClientHandler::mapParseErrorToHttpStatus(error::AppError err) {
    switch (err) {
        case error::kBadRequest:
            return http::kStatusBadRequest;
        case error::kRequestEntityTooLarge:
            return http::kStatusPayloadTooLarge;
        case error::kBadMethod:
            return http::kStatusNotImplemented;                     // 501
        case error::kBadHttpVersion:
            return http::kStatusHttpVersionNotSupported;            // 505
        case error::kMissingHost:
            return http::kStatusBadRequest;                         // 400
        case error::kInvalidContentLength:
        case error::kInvalidTransferEncoding:
        case error::kHasContentLengthAndTransferEncoding:
        case error::kcontainsNonDigit:
            return http::kStatusBadRequest;                         // 400
        case error::kBadLocationContext:
            return http::kStatusInternalServerError;                // 500
        case error::kUriTooLong:
            return http::kStatusUriTooLong;                         // 414
    }
    return http::kStatusBadRequest;                                 // 400
}
