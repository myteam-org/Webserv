#include "server/Server.hpp"
#include "utils/types/try.hpp"
#include "server/socket/SocketAddr.hpp"
#include "server/fileDescriptor/FdUtils.hpp"

Server::Server(const std::vector<ServerContext>& serverCtxs) 
    : serverCtxs_(serverCtxs), 
    epollNotifier_(), 
    connManager_(),
    dispatcher_(0),
    resolver_(serverCtxs_),
    endpointResolver_(vsByKey_) {   
    handlers_[FD_LISTENER]   = new ListenerHandler(this);
    handlers_[FD_CLIENT]     = new ClientHandler(this);
    handlers_[FD_CGI_STDIN]  = new CgiStdinHandler(this);
    handlers_[FD_CGI_STDOUT] = new CgiStdoutHandler(this);
}

Server::~Server() {
    delete handlers_[FD_LISTENER];
    delete handlers_[FD_CLIENT];
    delete handlers_[FD_CGI_STDIN];
    delete handlers_[FD_CGI_STDOUT];
}

types::Result<types::Unit, int> Server::init() {
    TRY(epollNotifier_.open());
    TRY(initVirtualServers());
    TRY(buildListeners());
    TRY(initDispatcher());
    return types::ok(types::Unit());
}

types::Result<types::Unit, int> Server::initVirtualServers() {
    for (size_t i = 0; i < serverCtxs_.size(); ++i) {
        const ServerContext& sc = serverCtxs_[i];

        const std::string key = makeEndpointKeyFromConfig(sc.getHost(), sc.getListen());
        for (std::map<std::string, VirtualServer*>::const_iterator it = vsByKey_.begin();
             it != vsByKey_.end(); ++it) {
            if (key == it->first || overlapsWildcard(key, it->first)) {
                return types::err<int>(EINVAL); // ワイルドカードと重複エラー
            }
        }
        VirtualServer* vs = new VirtualServer(sc, /*bindAddress=*/canonicalizeIp(sc.getHost()));
        vsByKey_[key] = vs;
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

types::Result<types::Unit,int> Server::initDispatcher() {
    dispatcher_ = new RequestDispatcher(endpointResolver_);
    return types::ok();
}

types::Result<types::Unit,int> Server::run() {
    for (;;) {
        types::Result<std::vector<EpollEvent>, int> r = epollNotifier_.wait(); // 200ms など
        if (r.isErr()) {
            return types::err<int>(r.unwrapErr());
        }
        const std::vector<EpollEvent>& evs = r.unwrap();
        for (size_t i = 0; i < evs.size(); ++i) {
            const EpollEvent& ev = evs[i];
            const int fd         = ev.getUserFd();
            const uint32_t mask  = ev.getEvents();
            FdEntry fdEntry;
            if (!fdRegister_.find(fd, &fdEntry)) {
                // epollDelClose(fd);
                continue;
            }
            IFdHandler* fdEventHandler = handlers_[fdEntry.kind];
            fdEventHandler->onEvent(fdEntry, mask);
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
        FdUtils::set_nonblock_and_cloexec(cfd);
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

std::string Server::makeEndpointKeyFromConfig(const std::string& hostName, unsigned short port) {
    const std::string ip = canonicalizeIp(hostName);
    return ip + ":" + u16toString(port);
}

std::string Server::u16toString(unsigned short port) {
    std::ostringstream oss;
    oss << port;
    return oss.str();
}

http::config::IConfigResolver&  Server::resolver() { 
    return resolver_;
}

void Server::armInOnly(int fd) {
    epollNotifier_.mod(fd, EPOLLIN | EPOLLRDHUP | EPOLLERR);
}
void Server::armOutOnly(int fd) {
    epollNotifier_.mod(fd, EPOLLOUT | EPOLLERR);
}
void Server::armInOut(int fd) {
    epollNotifier_.mod(fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR);
}

RequestDispatcher* Server::getDispatcher() const {
    return dispatcher_;
}

void Server::applyDispatchResult(Connection& c, const DispatchResult& dr) {
    const int cfd = c.getFd();
    if (dr.isNone()) {
        return;
    }
    if (dr.isArmOut()) {
        epollNotifier_.mod(cfd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR);
        return;
    }
    if (dr.isStartCgi()) {
        // CGI のパイプを epoll / registry に登録
        const int in_w  = dr.cgi.stdin_fd;   // サーバ→子に書く（OUT）
        const int out_r = dr.cgi.stdout_fd;  // 子→サーバから読む（IN）
        // モック（-1）のときは何もしない
        if (in_w >= 0 && out_r >= 0) {
            FdUtils::set_nonblock_and_cloexec(in_w);
            FdUtils::set_nonblock_and_cloexec(out_r);
            fdRegister_.add(in_w,  FD_CGI_STDIN,  &c);
            fdRegister_.add(out_r, FD_CGI_STDOUT, &c);
            epollNotifier_.add(in_w,  EPOLLOUT | EPOLLERR);
            epollNotifier_.add(out_r, EPOLLIN  | EPOLLERR);
        }
        // クライアント fd 側は通常 IN を維持（必要に応じて OUT も残してOK）
        // epollNotifier_.mod(cfd, EPOLLIN | EPOLLRDHUP | EPOLLERR);
        return;
    }
    if (dr.isDone()) {
        // CGI の後片付け：Connection から現在の CGI fd を引いて epoll/registry 解除
        // ※ Connection に以下の getter を用意しておくと楽:
        //    int getCgiInFd() const; int getCgiOutFd() const; bool hasCgi() const;
        if (/* c.hasCgi() */ true) {
            int in_w  = /* c.getCgiInFd()  */ -1;
            int out_r = /* c.getCgiOutFd() */ -1;

            if (in_w  >= 0) { 
                epollNotifier_.del(in_w);
                fdRegister_.erase(in_w);
                ::close(in_w);
            }
            if (out_r >= 0) { 
                epollNotifier_.del(out_r);
                fdRegister_.erase(out_r); 
                ::close(out_r); 
            }
            // c.clearCgi(); // ← Connection 側に用意しておく
        }
        // クライアント側は、まだ書くものがあれば OUT をアーム、無ければ IN のみ
        if (!c.getWriteBuffer().isEmpty()) {
            epollNotifier_.mod(cfd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR);
        } else {
            epollNotifier_.mod(cfd, EPOLLIN | EPOLLRDHUP | EPOLLERR);
        }
        return;
    }
    if (dr.isClose()) {
        // クライアント接続を閉じる（必要なら関連 CGI fd も掃除）
        // 1) CGI が生きていれば解除
        // if (c.hasCgi()) { ... 同上の del/erase/close ...; c.clearCgi(); }
        // 2) クライアント fd を epoll/registry から外す
        epollNotifier_.del(cfd);
        fdRegister_.erase(cfd);
        // 3) ConnectionManager から外して、fd close/メモリ解放
        connManager_.unregisterConnection(cfd);
        return;
    }
}

bool Server::overlapsWildcard(const std::string& a, const std::string& b) {

    size_t ca = a.find(':');
    size_t cb = b.find(':');
    if (ca == std::string::npos || cb == std::string::npos) {
        return false;
    }
    std::string ipa = a.substr(0, ca);
    std::string ipb = b.substr(0, cb);
    if (a.substr(ca+1) != b.substr(cb+1)) {
        return false;
    }
    // 同じポートで、どちらかが 0.0.0.0
    return (ipa == "0.0.0.0") || (ipb == "0.0.0.0");
}
