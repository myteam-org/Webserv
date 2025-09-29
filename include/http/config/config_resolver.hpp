#pragma once

#include "config/context/serverContext.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {
namespace config {

class IConfigResolver {
   public:
    virtual types::Result<const ServerContext*, error::AppError> chooseServer(
        const std::string& host) const = 0;
    virtual ~IConfigResolver() {}
};

class ConfigResolver : public IConfigResolver {
   public:
    explicit ConfigResolver(const std::vector<ServerContext>& servers)
        : servers_(servers) {}

    types::Result<const ServerContext*, error::AppError> chooseServer(
        const std::string& host) const {
        // host: "a.test:8080" → "a.test"
        std::size_t colon = host.find(':');
        std::string hostname = (colon != std::string::npos) ? host.substr(0, colon) : host;

        for (std::size_t i = 0; i < servers_.size(); ++i) {
            const std::vector<std::string>& serverNames = servers_[i].getServerNames();
            for (std::size_t j = 0; j < serverNames.size(); ++j) {
                if (serverNames[j] == hostname) {
                    // 一致する server_name を見つけた
                    return types::ok(&servers_[i]);
                }
            }
        }
        return types::err(error::kBadRequest);
    }

   private:
    std::vector<ServerContext> servers_;
};
}  // namespace config
}  // namespace http
