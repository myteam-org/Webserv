#include "config/context/serverContext.hpp"
#include "server/connection/ConnectionManager.hpp"
#include "server/EpollEventNotifier.hpp"
#include "http/virtual_server.hpp"
#include "server/socket/ServerSocket.hpp"

typedef std::pair<std::string, uint16_t> ListenerKey;

class Server {
    public:
        Server(const std::vector<ServerContext>&);
        ~Server();
        types::Result<void,int> init();

    private:
        std::vector<ServerContext> serverCtxs_;
        EpollEventNotifier epollNotifier_;
        ConnectionManager connManager_;
        std::map<std::string, VirtualServer*> virtualServers_;
        std::map<ListenerKey, ServerSocket*> listeners_; 
};
