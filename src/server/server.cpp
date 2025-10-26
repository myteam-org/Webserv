#include "server/Server.hpp"
#include "utils/types/try.hpp"
#include "server/socket/SocketAddr.hpp"
#include "server/fileDescriptor/FdUtils.hpp"
#include "utils/logger.hpp"
#include "utils/string.hpp"
#include "utils/types/error.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include "action/cgi_context.hpp"
#include "action/cgi_action.hpp"

Server::Server(const std::vector<ServerContext>& serverCtxs) 
    : serverCtxs_(serverCtxs), 
    epollNotifier_(), 
    connManager_(),
    dispatcher_(0),
    resolver_(serverCtxs_),
    endpointResolver_(vsByKey_) {   
    LOG_INFO("Server::Server: Server created");
    handlers_[FD_LISTENER]   = new ListenerHandler(this);
    handlers_[FD_CLIENT]     = new ClientHandler(this);
    handlers_[FD_CGI_STDIN]  = new CgiStdinHandler(this);
    handlers_[FD_CGI_STDOUT] = new CgiStdoutHandler(this);
}

Server::~Server() {
    LOG_INFO("Server::~Server: Server destroyed");
    delete handlers_[FD_LISTENER];
    delete handlers_[FD_CLIENT];
    delete handlers_[FD_CGI_STDIN];
    delete handlers_[FD_CGI_STDOUT];
}

types::Result<types::Unit, int> Server::init() {
    LOG_INFO("Server::init: Initializing server...");
    TRY(epollNotifier_.open());
    TRY(initVirtualServers());
    TRY(buildListeners());
    TRY(initDispatcher());
    LOG_INFO("Server::init: Server initialized successfully");
    return types::ok(types::Unit());
}

types::Result<types::Unit, int> Server::initVirtualServers() {
    LOG_INFO("Server::initVirtualServer: Initializing virtual servers...");
    for (size_t i = 0; i < serverCtxs_.size(); ++i) {
        const ServerContext& sc = serverCtxs_[i];

        const std::string key = makeEndpointKeyFromConfig(sc.getHost(), sc.getListen());
        for (std::map<std::string, VirtualServer*>::const_iterator it = vsByKey_.begin();
             it != vsByKey_.end(); ++it) {
            if (key == it->first || overlapsWildcard(key, it->first)) {
                LOG_ERROR("Server::initVirtualServer: Duplicate or overlapping server configuration for key: " + key);
                return types::err<int>(EINVAL); // ワイルドカードと重複エラー
            }
        }
        VirtualServer* vs = new VirtualServer(sc, /*bindAddress=*/canonicalizeIp(sc.getHost()));
        vsByKey_[key] = vs;
        LOG_INFO("Server::initVirtualServer: Virtual server created for key: " + key);
    }
    LOG_INFO("Server::initVirtualServer: Virtual servers initialized successfully");
    return types::ok();
}

types::Result<types::Unit,int> Server::buildListeners() {
    LOG_INFO("Server::buildListeners: Building listeners...");
    for (size_t i = 0; i < serverCtxs_.size(); ++i) {
        const ServerContext& sc = serverCtxs_[i];
        ListenerKey key;
        key.addr = canonicalizeIp(sc.getHost());
        key.port = sc.getListen();
        if (listeners_.find(key) != listeners_.end()) {
            continue;
        }
        LOG_INFO("Server::buildListeners: Creating listener for " + key.addr + ":" + u16toString(key.port));
        ServerSocket* ls = new ServerSocket();
        types::Result<int, int> r = ls->open(AF_INET, SOCK_STREAM, 0);
        if (r.isErr()) {
            LOG_ERROR("Server::buildListeners: Failed to open server socket");
            return types::err<int>(r.unwrapErr());
        }
        types::Result<int, int> r2 = ls->setReuseAddr(true);
        if (r2.isErr()) {
           LOG_ERROR("Server::buildListeners: Failed to set SO_REUSEADDR");
           return types::err<int>(r2.unwrapErr());
        };
        SocketAddr sa = SocketAddr::createIPv4(key.addr, key.port);
        types::Result<int, int> r3 = ls->bind(sa);
        if (r3.isErr()) {
           LOG_ERROR("Server::buildListeners: Failed to bind socket to " + key.addr + ":" + u16toString(key.port));
           return types::err<int>(r3.unwrapErr());
        };
        listeners_[key] = ls;
        TRY(ls->listen(SOMAXCONN));
        const int lfd = ls->getRawFd();
        epollNotifier_.add(lfd, EPOLLIN | EPOLLERR);
        fdRegister_.add(lfd, FD_LISTENER, 0);
        LOG_INFO("Server::buildListeners: Listener created and listening on " + key.addr + ":" + u16toString(key.port));
    }
    LOG_INFO("Listeners built successfully");
    return types::ok();
}

types::Result<types::Unit,int> Server::initDispatcher() {
    dispatcher_ = new RequestDispatcher(endpointResolver_);
    return types::ok();
}

types::Result<types::Unit,int> Server::run() {
    LOG_INFO("Server::run: Server run loop started");
    time_t lastSweep = std::time(0);
    for (;;) {
        types::Result<std::vector<EpollEvent>, int> r = epollNotifier_.wait(); // 200ms など
        if (r.isErr()) {
            LOG_ERROR("Server::run: epoll_wait failed");
            return types::err<int>(r.unwrapErr());
        }
        const std::vector<EpollEvent>& evs = r.unwrap();
        for (size_t i = 0; i < evs.size(); ++i) {
            const EpollEvent& ev = evs[i];
            const int fd         = ev.getUserFd();
            const uint32_t mask  = ev.getEvents();
            FdEntry fdEntry;
            if (!fdRegister_.find(fd, &fdEntry)) {
                LOG_WARN("Server::run: No FdEntry found for fd: " + utils::toString(fd));
                // epollDelClose(fd);
                continue;
            }
            IFdHandler* fdEventHandler = handlers_[fdEntry.kind];
            fdEventHandler->onEvent(fdEntry, mask);
        }
        const time_t now = std::time(0);
        if (now - lastSweep >= 1) {  // 1秒ごとに一度だけ呼ぶ
            sweepTimeouts();
            lastSweep = now;
        }
    }
}

void Server::acceptLoop(int lfd) {
    LOG_INFO("Server::acceptLoop: Accepting new connections on fd: " + utils::toString(lfd));
    for (;;) {
        SocketAddr peer = SocketAddr::makeEmpty();
        socklen_t len = peer.length();
        int cfd = ::accept(lfd, peer.raw(), &len);
        peer.setLength(len);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            LOG_ERROR("Server::acceptLoop: accept failed");
            break;
        }
        peer.setLength(len);
        LOG_INFO("Server::acceptLoop: Accepted new connection on fd: " + utils::toString(cfd));
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
        types::Result<CgiFds, error::SystemError> res = spawnCgiFromPrepared(c);
        if (res.isErr()) {
            LOG_ERROR("Server::applyDispatchResult: CGI FD initialization error.");
            DispatchResult err = getDispatcher()->emitError(c, http::kStatusBadGateway, "Bad Gateway");
            applyDispatchResult(c, err);
            return;
        }
        registerCgiFds(c, res.unwrap());
        c.markFrontDispatched();
        return;
    }
    if (dr.isCgiCloseIn()) {
        if (c.isCgiActive()) {
            CgiContext* ctx = c.getCgi();
            if (ctx->getFdIn() >= 0) {
                epollNotifier_.del(ctx->getFdIn());
                fdRegister_.erase(ctx->getFdIn());
                FdUtils::safe_fd_close(ctx->getFdIn());
                ctx->setFdIn(-1);
            }
        }
        return;
    }
    if (dr.isCgiCloseOut()) {
        if (c.isCgiActive()) {
            CgiContext* ctx = c.getCgi();
            if (ctx->getFdOut() >= 0) {
                epollNotifier_.del(ctx->getFdOut());
                fdRegister_.erase(ctx->getFdOut());
                ::close(ctx->getFdOut());
                ctx->setFdOut(-1);
            }
            // 子の回収はここで（WNOHANG）
            int st=0;
            (void)::waitpid(ctx->getPid(), &st, WNOHANG);
        }
        return;
    }
    if (dr.isCgiAbort()) {
        if (c.isCgiActive()) {
            cleanupConnectionCgi(c); // ← 両FDの del/erase/close + ctx delete をまとめるユーティリティ
        }
        // レスポンスはすでに failCgi() が enqueue 済み。送信待ちにする
        if (!c.getWriteBuffer().isEmpty())
            epollNotifier_.mod(cfd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR);
        else
            epollNotifier_.mod(cfd, EPOLLIN | EPOLLRDHUP | EPOLLERR);
        return;
    }
    if (dr.isDone()) {
        if (c.isCgiActive()) {
            cleanupConnectionCgi(c);
        }
        if (!c.getWriteBuffer().isEmpty()) {
            epollNotifier_.mod(cfd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR);
        } else {
            epollNotifier_.mod(cfd, EPOLLIN | EPOLLRDHUP | EPOLLERR);
        }
        return;
    }
    if (dr.isClose()) {
        if (c.isCgiActive()) {
            cleanupConnectionCgi(c);
        }
        epollNotifier_.del(cfd);
        fdRegister_.erase(cfd);
        connManager_.unregisterConnection(cfd);
        return;
    }
}

void Server::cleanupConnectionCgi(Connection& c) {
    int in_w  = c.getCgi()->getFdIn(); 
    int out_r =  c.getCgi()->getFdOut();
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
    c.clearCgi();
}

types::Result<CgiFds, error::SystemError> Server::spawnCgiFromPrepared(Connection& c) {
    CgiFds ret;
    ret.stdin_fd = -1;
    ret.stdout_fd = -1;
    PreparedCgi* prepared_ctx = c.takePreparedCgi();
    if (!prepared_ctx)
        return ERR(error::kUnknownError);
    int in_pipe[2]  = { -1, -1 };
    int out_pipe[2] = { -1, -1 };
    if (setupPipeForCgi(in_pipe, out_pipe).isErr()) {
        delete prepared_ctx;
        return ERR(error::kPipeCreateFailed);
    }
    FdUtils::set_nonblock_and_cloexec(in_pipe[1]);
    FdUtils::set_nonblock_and_cloexec(out_pipe[0]);
    std::vector<char*> argvp, envp;
    to_c_argv(prepared_ctx->argv, argvp);
    to_c_argv(prepared_ctx->env,  envp);
    pid_t pid = ::fork();
    if (pid < 0) {
        LOG_WARN("Server::spawnCgiFromPrepared: fork failed");
        closeAllPipeFds(in_pipe, out_pipe);
        delete prepared_ctx;
        return ERR(error::kForkFailed);
    }
    if (pid == 0) {
        ::dup2(in_pipe[0],  STDIN_FILENO);
        ::dup2(out_pipe[1], STDOUT_FILENO);
        ::dup2(out_pipe[1], STDERR_FILENO);
        FdUtils::safe_fd_close(in_pipe[1]);
        FdUtils::safe_fd_close(out_pipe[0]); 
        ::execve(prepared_ctx->argv[0].c_str(), &argvp[0], &envp[0]);
        LOG_ERROR(::strerror(errno));
        LOG_ERROR("Server::spawnCgiFromPrepared: CGI execution error");
        _exit(127);
    }
    FdUtils::safe_fd_close(in_pipe[0]);
    FdUtils::safe_fd_close(out_pipe[1]);
    CgiContext* ctx = new CgiContext();
    ctx->setProc(pid, in_pipe[1], out_pipe[0],std::time(0));
    ctx->setStdinBody(prepared_ctx->stdinBody);
    ctx->setOwner(prepared_ctx->owner, prepared_ctx->parseFn);
    c.setCgi(ctx);
    ret.stdin_fd  = ctx->getFdIn();
    ret.stdout_fd = ctx->getFdOut();
    delete prepared_ctx;
    return types::ok(ret);
}

void execChildProcess(int in_pipe[2], int out_pipe[2], PreparedCgi* prepared_ctx, std::vector<char*> argvp,  std::vector<char*> envp) {
    ::dup2(in_pipe[0],  STDIN_FILENO);
    ::dup2(out_pipe[1], STDOUT_FILENO);
    ::dup2(out_pipe[1], STDERR_FILENO);
    FdUtils::safe_fd_close(in_pipe[1]);
    FdUtils::safe_fd_close(out_pipe[0]); 
    ::execve(prepared_ctx->argv[0].c_str(), &argvp[0], &envp[0]);
    _exit(127);
}

types::Result<types::Unit, error::SystemError> Server::setupPipeForCgi(int in_pipe[2], int out_pipe[2]) {
    if (::pipe(in_pipe) < 0) {
        LOG_WARN("Server::setupPipeForCgi: in pipe creation failed");
        return ERR(error::kPipeCreateFailed);
    }
    if (::pipe(out_pipe) < 0) {
        LOG_WARN("Server::setupPipeForCgi: out pipe creation failed");
        FdUtils::safe_fd_close(in_pipe[0]);
        FdUtils::safe_fd_close(in_pipe[1]);
        return ERR(error::kPipeCreateFailed);
    }
    return types::ok(types::Unit());
}

void Server::closeAllPipeFds(int in_pipe[2], int out_pipe[2]) {
    FdUtils::safe_fd_close(in_pipe[0]);
    FdUtils::safe_fd_close(in_pipe[1]);
    FdUtils::safe_fd_close(out_pipe[0]);
    FdUtils::safe_fd_close(out_pipe[1]);
}

void  Server::to_c_argv(const std::vector<std::string>& src, std::vector<char*>& dst) {
    dst.clear();
    dst.reserve(src.size() + 1);
    for (size_t i = 0; i < src.size(); ++i)
        dst.push_back(const_cast<char*>(src[i].c_str()));
    dst.push_back(0);
}

void Server::registerCgiFds(Connection& c, const CgiFds& fds) {
    if (fds.stdout_fd >= 0) {
        fdRegister_.add(fds.stdout_fd, FD_CGI_STDOUT, &c);
        epollNotifier_.add(fds.stdout_fd, EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR);
    }
    if (fds.stdin_fd >= 0) {
        fdRegister_.add(fds.stdin_fd, FD_CGI_STDIN, &c);
        // ボディがある時だけ EPOLLOUT
        const bool needWrite = c.front().getBody().size() > 0;
        if (needWrite)
            epollNotifier_.add(fds.stdin_fd, EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR);
        else
            ::close(fds.stdin_fd);
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

void Server::sweepTimeouts() {
    const time_t now = std::time(0);
    std::map<int, Connection*>& conns = connManager_.getAllConnections();

    for (std::map<int, Connection*>::iterator it = conns.begin(); it != conns.end(); ++it) {
        Connection* c = it->second;
        if (now - c->getLastRecv() > Connection::kTimeoutThresholdSec) {
            LOG_INFO("timeout fd=" + utils::toString(c->getFd()));
            epollNotifier_.del(c->getFd());
            connManager_.unregisterConnection(c->getFd());
            delete c;
        }
        if (c->isCgiActive()) {
            CgiContext* cgictx = c->getCgi();
            const time_t elapsed = now - cgictx->getLastRecv();
            if (elapsed > CgiContext::kCGITimeoutThresholdSec) {
                LOG_WARN("CGI timeout pid=" + utils::toString(cgictx->getPid()));
                kill(cgictx->getPid(), SIGKILL);
                waitpid(cgictx->getPid(), NULL, WNOHANG);
                DispatchResult err = getDispatcher()->emitError(*c, http::kStatusGatewayTimeout, "Gateway Timeout");
            }
        }
    }
}
