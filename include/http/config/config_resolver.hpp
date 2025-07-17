#pragma once

#include "config/context/serverContext.hpp"

namespace http {
namespace config {

class IConfigResolver {
   public:
    virtual const ServerContext& choseServer(const std::string& host) const = 0;
    virtual ~IConfigResolver() {}
};

class ConfigResolver : public IConfigResolver {
   public:
    explicit ConfigResolver(const std::vector<ServerContext>& servers) : servers_(servers) {}

    const ServerContext& choseServer(const std::string& host) const {
        for (std::size_t i = 0; i < servers_.size(); ++i) {
            if (servers_[i].getHost() == host) {
                return servers_[i];
            }
        }
        return servers_[0];
    }
   private:
    std::vector<ServerContext> servers_;
};
} // namespace config
} // namespace http