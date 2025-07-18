#pragma once

#include "config/config.hpp"
#include "router/router.hpp"

class VirtualServer {
public:
    explicit VirtualServer(const ServerContext &serverConfig, const std::string &bindAddress);

    const ServerContext &getServerConfig() const;
    http::Router &getRouter();

    void registerHandlers(const LocationContext &location);

private:
    ServerContext serverConfig_;
    std::string bindAddress_;
    http::Router router_;

    void setupRouter();

    friend class VirtualServerResolver;
};
