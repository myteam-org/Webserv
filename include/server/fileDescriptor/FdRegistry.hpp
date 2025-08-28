#pragma once
#include "server/connection/Connection.hpp"
#include <map>


enum FdKind { FD_LISTENER, FD_CLIENT, FD_CGI_STDIN, FD_CGI_STDOUT };

struct FdEntry {
    int fd;
    FdKind kind;
    Connection* conn;
};

class FdRegistry {
public:
    bool add(int fd, FdKind k, Connection* c);
    void erase(int fd);
    bool is(int fd, FdKind k) const;
    bool find(int fd, FdEntry* out) const;
    Connection* getConn(int fd) const;
private:
    std::map<int, FdEntry> table_;
};
