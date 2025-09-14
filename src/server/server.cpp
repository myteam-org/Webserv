#include "server/Server.hpp"
#include "utils/types/try.hpp"

#include "server/socket/SocketAddr.hpp"

Server::Server(const std::vector<ServerContext>& serverCtxs) 
    : serverCtxs_(serverCtxs), 
    epollNotifier_(), 
    connManager_(),
    resolver_(serverCtxs_) {
    handlers_[FD_LISTENER]   = new ListenerHandler(this);
    handlers_[FD_CLIENT]     = new ClientHandler(this);
    handlers_[FD_CGI_STDIN]  = new CgiStdinHandler(this);
    handlers_[FD_CGI_STDOUT] = new CgiStdoutHandler(this);
}

types::Result<types::Unit, int> Server::init() {
    TRY(epollNotifier_.open());
    TRY(initVirtualServers());
    TRY(buildListeners());
    return types::ok();
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
        TRY(ls->listen(SOMAXCONN));
        const int lfd = ls->getRawFd();
        epollNotifier_.add(lfd, EPOLLIN | EPOLLERR);
        fdRegister_.add(lfd, FD_LISTENER, 0);
    }
    return types::ok();
}

// todo リファクタリングを行い、各　fd の状態に対するディスパッチする。
types::Result<types::Unit,int> Server::run() {
    for (;;) {
        types::Result<std::vector<EpollEvent>, int> r = epollNotifier_.wait(); // 200ms など
        if (r.isErr()) return types::err<int>(r.unwrapErr());

        const std::vector<EpollEvent>& evs = r.unwrap();
        for (size_t i = 0; i < evs.size(); ++i) {
            const EpollEvent& ev = evs[i];
            const int fd         = ev.getUserFd();
            const uint32_t mask  = ev.getEvents();
            FdEntry ent;
            if (!fdRegister_.find(fd, &ent)) {
                // epollDelClose(fd);
                continue;
            }
            IFdHandler* fdEventHandler = handlers_[ent.kind];
            fdEventHandler->onEvent(ent, mask);
            // if (fdRegister_.is(fd, FD_LISTENER)) {
            //     if (mask & (EPOLLERR | EPOLLHUP)) {
            //         continue;
            //     }
            //     acceptLoop(fd);        // 非ブロッキング accept を EAGAIN まで
            //     continue;
            // }
            // if (fdRegister_.is(fd, FD_CGI_STDIN)) {       // CGI stdin (書き込み側=サーバから見てOUT)
            //     Connection* c = connManager_.getByCgiIn(fd);
            //     if (!c) { safeDropUnknownFd(fd); continue; }
            //     if (mask & EPOLLOUT) c->onCgiStdinWritable();
            //     if (mask & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) c->onCgiPipeHangup();
            //     continue;
            // }
            // if (fdRegister_.is(fd, FD_CGI_STDOUT)) {      // CGI stdout (読み取り側=IN)
            //     Connection* c = connManager_.getByCgiOut(fd);
            //     if (!c) { safeDropUnknownFd(fd); continue; }
            //     if (mask & EPOLLIN) c->onCgiStdoutReadable();
            //     if (mask & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) c->onCgiPipeHangup();
            //     continue;
            // }

            // types::Result<Connection *, std::string> conRes = connManager_.getConnectionByFd(fd);
            // if (conRes.isErr()) {
            //      //safedrop();
            //      continue;
            // }
            // Connection* conn = conRes.unwrap();
            // if (mask & (EPOLLERR | EPOLLHUP)) { 
            //     c->onHangup();
            //     continue; 
            // }
            // if (mask & EPOLLRDHUP) { 
            //     c->onPeerHalfClose();
            // }
            // if (mask & EPOLLIN) { // RequestReader→pending_
            //     c->onReadable();
            // }
            // if (conn->hasPending()) {
            //     dispatcher_.ensureVhost(*conn); // ヘッダ揃った後に一度だけ
            //     DispatchResult dr = dispatcher_.dispatchNext(*conn);
            //     if (dr.next == DispatchResult::kArmOut) {
            //         epollNotifier_.mod(fd, EPOLLIN|EPOLLOUT|EPOLLRDHUP|EPOLLERR);
            //     } else if (dr.next == DispatchResult::kStartCgi) {
            // // CGI の fd を epoll 登録 & レジストリ登録
            //     fdRegister_.add(dr.cgi_in_w,  FD_CGI_STDIN,  conn);
            //     fdRegister_.add(dr.cgi_out_r, FD_CGI_STDOUT, conn);
            //     epollNotifier_.add(dr.cgi_in_w,  EPOLLOUT | EPOLLERR);
            //     epollNotifier_.add(dr.cgi_out_r, EPOLLIN  | EPOLLERR);
            //     }
            // }
            // if (mask & EPOLLOUT) { 
            //     c->onWritable(); 
            // }
        }
        // sweepTimeouts(); // ヘッダ/ボディ/CGI ごとの壁時計タイムアウトで close
    }
}

void Server::acceptLoop(int lfd) {
    for (;;) {
        SocketAddr peer = SocketAddr::makeEmpty();
        socklen_t len = peer.length();
        int cfd = ::accept(lfd, peer.raw(), &len);
        peer.setLength(len);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            // Todo : emit log.
            break;
        }
        peer.setLength(len);
        set_nonblock_and_cloexec(cfd);
        Connection* conn = new Connection(cfd, peer, this->resolver());
        connManager_.registerConnection(conn);
        epollNotifier_.add(cfd, EPOLLIN | EPOLLRDHUP | EPOLLERR);
        fdRegister_.add(cfd, FD_CLIENT, conn);
    }
}

// hostNameを正規化して dotted decimal string を返す
std::string canonicalizeIp(const std::string& hostName) {
    if (hostName.empty() || hostName == "*" || hostName == "0.0.0.0")
        return "0.0.0.0";
    SocketAddr sa = SocketAddr::createIPv4(hostName, 0);
    return sa.getAddress();
}

http::config::IConfigResolver&  Server::resolver() { 
    return resolver_;
}