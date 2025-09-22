#pragma once

#include "config/config.hpp"
#include "http/handler/router/builder.hpp"
#include "http/handler/router/router.hpp"

class VirtualServer {
   public:
    VirtualServer(const ServerContext &serverConfig,
                  const std::string &bindAddress);
    ~VirtualServer();

    const ServerContext &getServerConfig() const;
    http::Router &getRouter();

   private:
    static void registerHandlers(http::RouterBuilder &routerBuilder,
                                 const LocationContext &locationContext);
    void setupRouter();

    const ServerContext serverConfig_;
    const std::string bindAddress_;
    http::Router *router_;
};
