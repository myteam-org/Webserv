#include "server/Server.hpp"
#include "utils/types/try.hpp"

#include "SocketAddr.hpp"

Server::Server(const std::vector<ServerContext>& serverCtxs) 
    : serverCtxs_(serverCtxs), 
    epollNotifier_(), 
    connManager_() {}

types::Result<types::Unit, int> Server::init() {
    TRY(epollNotifier_.open());
    TRY(initVirtualServers());
    TRY(buildListeners());
}


types::Result<types::Unit, int> Server::initVirtualServers() {
    for (size_t i = 0; i < serverCtxs_.size(); ++i) {
        const ServerContext& serverCtx = serverCtxs_[i];
        //Todo : virtualServer で bindAddress の使用が必要か否か。vhost の要件がないので単純にここで検索要件としてもよいかも
        VirtualServer* virtualServer = new VirtualServer(serverCtx, /*bindAddress=*/"");
        virtualServers_[serverCtx.getServerName()] = virtualServer;
    }
    return types::ok();
}

// Todo: VirtualServer が複数の listen ポートを許容する場合、2 重ループにて処理する必要がある。
types::Result<types::Unit,int> Server::buildListeners() {
    for (size_t i = 0; i < serverCtxs_.size(); ++i) {
        const ServerContext& sc = serverCtxs_[i];
        // for (size_t j = 0; j < sc.binds.size(); ++j) {
            ListenerKey key;
            key.addr = canonicalizeIp(sc.getHost());
            key.port = sc.getListen();
            if (listeners_.find(key) != listeners_.end()) {
                continue;
            }
            ServerSocket* ls = new ServerSocket();
            types::Result<int, int> r = ls->open(AF_INET, SOCK_STREAM, 0);
            if (r.isErr()) {
                return types::err<int>(r.unwrapErr());
            }
            types::Result<int, int> r2 = ls->setReuseAddr(true);
            if (r2.isErr()) {
               return types::err<int>(r2.unwrapErr());
            };
            SocketAddr sa = SocketAddr::createIPv4(key.addr, key.port);
            types::Result<int, int> r3 = ls->bind(sa);
            if (r3.isErr()) {
               return types::err<int>(r3.unwrapErr());
            };
            listeners_[key] = ls;
        // }
    }
    return types::ok();
}


types::Result<types::Unit,int> Server::run() {
    for (;;) {
        types::Result<std::vector<EpollEvent>, int> r = epollNotifier_.wait(); // 200ms など
        if (r.isErr()) return types::err<int>(r.unwrapErr());

        const std::vector<EpollEvent>& evs = r.unwrap();
        for (size_t i = 0; i < evs.size(); ++i) {
            const EpollEvent& ev = evs[i];
            const int fd         = ev.getUserFd();     // ← data.fd
            const uint32_t mask  = ev.getEvents();

            // 1) リスナFD？
            if (isListenerFd(fd)) {
                if (mask & (EPOLLERR | EPOLLHUP)) {
                    // ここはログだけ。致命的でなければ継続
                    continue;
                }
                acceptLoop(fd);        // 非ブロッキング accept を EAGAIN まで
                continue;
            }

            // 2) CGI パイプ？
            if (isCgiInFd(fd)) {       // CGI stdin (書き込み側=サーバから見てOUT)
                Connection* c = connManager_.getByCgiIn(fd);
                if (!c) { safeDropUnknownFd(fd); continue; }
                if (mask & EPOLLOUT) c->onCgiStdinWritable();
                if (mask & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) c->onCgiPipeHangup();
                continue;
            }
            if (isCgiOutFd(fd)) {      // CGI stdout (読み取り側=IN)
                Connection* c = connManager_.getByCgiOut(fd);
                if (!c) { safeDropUnknownFd(fd); continue; }
                if (mask & EPOLLIN) c->onCgiStdoutReadable();
                if (mask & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) c->onCgiPipeHangup();
                continue;
            }

            // 3) クライアントFD
            Connection* c = connManager_.getConnectionByFd(fd);
            if (!c) { safeDropUnknownFd(fd); continue; }

            if (mask & (EPOLLERR | EPOLLHUP)) { c->onHangup(); continue; }
            if (mask & EPOLLRDHUP)            { c->onPeerHalfClose(); /* 続行可 */ }
            if (mask & EPOLLIN)               { c->onReadable(); }   // RequestReader→pending_
            if (mask & EPOLLOUT)              { c->onWritable(); }   // WriteBuffer.flush()
        }
        sweepTimeouts(); // ヘッダ/ボディ/CGI ごとの壁時計タイムアウトで close
    }
    // not reached
}

void Server::acceptLoop(int lfd) {
    for (;;) {
        int cfd = ::accept(lfd, 0, 0);
        if (cfd < 0) break;                // 非ブロッキング: EAGAIN 相当で抜け
        set_nonblock_and_cloexec(cfd);

        Connection* c = connManager_.create(cfd, lk.port /*localPort*/, resolver_.get());
        c->enterReceiving();               // state = RECEIVING, pending_ 空で開始

        // epoll 登録: IN | RDHUP | ERR
        EventData *ed = new EventData();
        ed->kind = CLIENT; ed->fd = cfd; ed->conn = c;
        epollNotifier_.add(cfd, EPOLLIN | EPOLLRDHUP | EPOLLERR, ed);
    }
}

bool Server::isListenerFd(int fd) const {
    return fd2listener_.find(fd) != fd2listener_.end();
}
bool Server::isCgiInFd(int fd) const  { return connManager_.hasCgiIn(fd); }
bool Server::isCgiOutFd(int fd) const { return connManager_.hasCgiOut(fd); }


// void Server::handleClientEvent(Connection* c, uint32_t ee) {
//     if (ee & (EPOLLERR|EPOLLHUP)) { c->onHangup(); return; }
//     if (ee & EPOLLRDHUP)           { c->onPeerHalfClose(); /* 以後 OUT だけ継続可 */ }
//     if (ee & EPOLLIN)              { c->onReadable(); }    // RequestReader を進める
//     if (ee & EPOLLOUT)             { c->onWritable(); }    // WriteBuffer.flush()
// }


// hostNameを正規化して dotted decimal string を返す
std::string canonicalizeIp(const std::string& hostName) {
    if (hostName.empty() || hostName == "*" || hostName == "0.0.0.0")
        return "0.0.0.0";
    SocketAddr sa = SocketAddr::createIPv4(hostName, /*port=*/0);
    return sa.getAddress();
}

bool Server::isListenerFd(int fd) const {
    return fd2listener_.find(fd) != fd2listener_.end();
}
bool Server::isCgiInFd(int fd) const  { return connManager_.hasCgiIn(fd); }
bool Server::isCgiOutFd(int fd) const { return connManager_.hasCgiOut(fd); }

