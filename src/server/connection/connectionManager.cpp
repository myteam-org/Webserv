#include "server/connection/ConnectionManager.hpp"

ConnectionManager::ConnectionManager() {
}

types::Result<Connection*, std::string> ConnectionManager::getConnectionByFd(int fd) const {
    std::map<int, Connection*>::const_iterator it = connectionMap_.find(fd);
    if (it == connectionMap_.end()) {
        return types::err<std::string>("connection not found");
    }
    return types::ok<Connection*>(it->second);
}

types::Result<int, int> ConnectionManager::registerConnection(Connection* conn) {
    const int fd = conn->getConnSock().getRawFd();
    if (fd < 0) {
        return types::err<int>(-1); // fd is invalid
    }
    if (connectionMap_.find(fd) != connectionMap_.end()) {
        return types::err<int>(-2); // duplicate error
    }
    connectionMap_[fd] = conn;
    return types::ok<int>(fd);
}

types::Result<int, int> ConnectionManager::unregisterConnection(int fd) {
    std::map<int, Connection*>::iterator it = connectionMap_.find(fd);
    if (it == connectionMap_.end()) {
        return types::err<int>(-1); //connection is not registered
    }
    delete it->second;
    connectionMap_.erase(it);
    return types::ok<int>(fd);
}

ConnectionManager::~ConnectionManager() {
    for (std::map<int, Connection*>::iterator it = connectionMap_.begin();
        it != connectionMap_.end(); ++it) {
        delete it->second;
    }
    connectionMap_.clear();
}

std::map<int, Connection*>& ConnectionManager::getAllConnections() {
    return connectionMap_;
}