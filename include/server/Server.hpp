#pragma once
#include "config/context/serverContext.hpp"
#include "server/connection/ConnectionManager.hpp"
#include "server/EpollEventNotifier.hpp"
#include "http/virtual_server.hpp"
#include "server/socket/ServerSocket.hpp"
#include "server/fileDescriptor/FdRegistry.hpp"
#include "io/handler/IFdHandler.hpp"
#include "io/handler/ClientHandler.hpp"
#include "io/handler/CgiStdinHandler.hpp"
#include "io/handler/CgiStdoutHandler.hpp"
#include "io/handler/Listenhandler.hpp"
#include "http/config/config_resolver.hpp"
#include "server/dispatcher/RequestDispatcher.hpp"
#include "server/resolver/EndpointResolver.hpp"


struct ListenerKey {
    std::string addr;  // 正規化済みの数値IP（例: "127.0.0.1" or "0.0.0.0"）
    uint16_t    port;

    bool operator<(const ListenerKey& rhs) const {
        if (addr != rhs.addr) return addr < rhs.addr;
        return port < rhs.port;
    }
};

class Server {
    public:
        Server(const std::vector<ServerContext>&);
        ~Server();
        types::Result<types::Unit, int> init();
        types::Result<types::Unit, int> buildListeners();
        types::Result<types::Unit, int> run();
        types::Result<types::Unit, int> initVirtualServers();
        types::Result<types::Unit, int> wireListenersToServers();
        types::Result<types::Unit, int> initDispatcher();
        void acceptLoop(int lfd);
        http::config::IConfigResolver& resolver();
        void applyDispatchResult(Connection& c, const DispatchResult& dr);
        void armInOnly(int fd);
        void armOutOnly(int fd);
        void armInOut(int fd);
        RequestDispatcher* getDispatcher() const;
        static const time_t kTimeoutThresholdSec = 30;
        static const time_t kCGITimeoutThresholdSec = 15;
    private:
        std::vector<ServerContext> serverCtxs_;
        EpollEventNotifier epollNotifier_;
        ConnectionManager connManager_;
        FdRegistry fdRegister_;
        IFdHandler* handlers_[4];
        std::map<std::string, VirtualServer*> vsByKey_;
        RequestDispatcher *dispatcher_;
        std::map<ListenerKey, ServerSocket*> listeners_; 
        http::config::ConfigResolver resolver_;
        EndpointResolver endpointResolver_;
        bool overlapsWildcard(const std::string& a, const std::string& b);
        std::string makeEndpointKeyFromConfig(const std::string& hostName, unsigned short port);
        std::string u16toString(unsigned short port);
        void registerCgiFds(Connection& c, const CgiFds& fds);
        types::Result<CgiFds, error::SystemError> spawnCgiFromPrepared(Connection& c);
        types::Result<types::Unit, error::SystemError> setupPipeForCgi(int in_pipe[2], int out_pipe[2]);
        static void closeAllPipeFds(int in_pipe[2], int out_pipe[2]);
        static void to_c_argv(const std::vector<std::string>& src, std::vector<char*>& dst);
        void cleanupConnectionCgi(Connection& c);
        void sweepTimeouts();
};

std::string canonicalizeIp(const std::string& hostName);
