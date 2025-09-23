#include "server/fileDescriptor/FdRegistry.hpp"


bool FdRegistry::add(int fd, FdKind k, Connection* c) {
    if (fd < 0) {
        return false;
    }
    std::map<int, FdEntry>::iterator it = table_.find(fd);
    if (it != table_.end()) {
        return false;
    }
    FdEntry e;
    e.fd   = fd;
    e.kind = k;
    e.conn = c; // LISTENER の場合は 0 を渡す
    table_.insert(std::make_pair(fd, e));
    return true;
}

void FdRegistry::erase(int fd) {
    std::map<int, FdEntry>::iterator it = table_.find(fd);
    if (it != table_.end()) {
        table_.erase(it);
    }
}

bool FdRegistry::is(int fd, FdKind k) const {
    std::map<int, FdEntry>::const_iterator it = table_.find(fd);
    if (it == table_.end()) return false;
    return it->second.kind == k;
}

Connection* FdRegistry::getConn(int fd) const {
    std::map<int, FdEntry>::const_iterator it = table_.find(fd);
    if (it == table_.end()) return 0;
    return it->second.conn;
}

bool FdRegistry::find(int fd, FdEntry* out) const {
    std::map<int,FdEntry>::const_iterator it = table_.find(fd);
    if (it == table_.end()) {
        return false;
    }
    if (out) {
        *out = it->second;
    }
    return true;
}
