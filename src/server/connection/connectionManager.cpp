#include "ConnectionManager.hpp"

ConnectionManager::ConnectionManager() {
}

types::Result<const Connection&, std::string> ConnectionManager::getConnectionByFd(FileDescriptor& fd) const {
    const types::Option<int> fdRes = fd.getFd();
    if (!fdRes.canUnwrap()) {
        return types::err<std::string>("invalid Filedescriptor");
    }
    std::map<int, Connection*>::const_iterator it = connectionMap_.find(fdRes.unwrap());
     if (it == connectionMap_.end()) {
        return types::err<std::string>("connection not found");
    }
    return types::ok<const Connection&>(*(it->second));
}

types::Result<int, int> ConnectionManager::registerConnection(Connection& conn) {
    int fd = -1;
    const int fd = conn.getConnSock().getRawFd();
    if (fd < 0) {
        return types::err<int>(-1); // fd is invalid
    }
    if (connectionMap_.find(fd) != connectionMap_.end()) {
        return types::err<int>(-2); // duplicate error
    }
    connectionMap_[fd] = &conn;
    return types::ok<int>(fd);
}

types::Result<int, int> ConnectionManager::unregisterConnection(int fd) {
    std::map<int, Connection*>::iterator it = connectionMap_.find(fd);
    if (it == connectionMap_.end()) {
        return types::err<int>(-1); //connection is not registered
    }
    connectionMap_.erase(it);
    return types::ok<int>(fd);
}

ConnectionManager::~ConnectionManager() {
    connectionMap_.clear();
}