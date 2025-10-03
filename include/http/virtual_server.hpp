#pragma once

#include "config/config.hpp"
#include "http/handler/router/builder.hpp"
#include "http/handler/router/router.hpp"

// 仮想サーバ選択はserver_namesのみ。host_はバインド用のみ。
class VirtualServer {
   public:
    VirtualServer(const ServerContext &serverConfig,
                  const std::string &bindAddress);
    ~VirtualServer();

    const ServerContext &getServerConfig() const;
    http::Router &getRouter();

    bool matchesHost(const std::string &host) const;
    static VirtualServer* findByHost(const std::vector<VirtualServer*>& servers, const std::string& host);

   private:
    void setupRouter();

    const ServerContext serverConfig_;
    const std::string bindAddress_;
    http::Router *router_;
};
