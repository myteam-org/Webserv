#include "server/Server.hpp"

Server::Server(const std::vector<ServerContext>& serverCtxs) 
    : serverCtxs_(serverCtxs), 
    epollNotifier_(), 
    connManager_() {}

types::Result<void, int> Server::init() {
    
}

