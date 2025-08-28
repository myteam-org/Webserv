#include "config/context/serverContext.hpp"
#include "server/connection/ConnectionManager.hpp"
#include "server/EpollEventNotifier.hpp"
#include "http/virtual_server.hpp"
#include "server/socket/ServerSocket.hpp"
#include "server/fileDescriptor/FdRegistry.hpp"


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
        types::Result<types::Unit,int> init();
        types::Result<types::Unit,int> buildListeners();
        types::Result<types::Unit,int> run();
        types::Result<types::Unit,int> initVirtualServers();
        types::Result<types::Unit,int> wireListenersToServers();
        void acceptLoop(int lfd);

    private:
        std::vector<ServerContext> serverCtxs_;
        EpollEventNotifier epollNotifier_;
        ConnectionManager connManager_;
        FdRegistry fdRegister_;
        std::map<std::string, VirtualServer*> virtualServers_;
        std::map<ListenerKey, ServerSocket*> listeners_; 
};

std::string canonicalizeIp(const std::string& hostName);