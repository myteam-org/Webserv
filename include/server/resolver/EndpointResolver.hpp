#pragma once
#include <map>
#include <string>
#include "http/virtual_server.hpp"
#include "server/socket/SocketAddr.hpp"

class EndpointResolver {
public:
    explicit EndpointResolver(const std::map<std::string, VirtualServer*>& vsByKey);
    VirtualServer* resolveByFd(int cfd) const ;

private:
    const std::map<std::string, VirtualServer*>& vsByKey_;
    static std::string endpointKeyFromFd(int fd);
};