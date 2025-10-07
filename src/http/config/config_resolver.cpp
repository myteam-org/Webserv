#include "http/config/config_resolver.hpp"
#include "utils/logger.hpp"
#include <vector>

namespace http {
namespace config {

ConfigResolver::ConfigResolver(const std::vector<ServerContext>& servers)
    : servers_(servers) {}

ConfigResolver::~ConfigResolver() {}

types::Result<const ServerContext*, error::AppError>
ConfigResolver::chooseServer(const std::string& host) const {
    std::size_t colon = host.find(':');
    std::string hostname = (colon != std::string::npos) ? host.substr(0, colon) : host;

    for (std::size_t i = 0; i < servers_.size(); ++i) {
        const std::vector<std::string>& serverNames = servers_[i].getServerNames();
        for (std::size_t j = 0; j < serverNames.size(); ++j) {
            if (serverNames[j] == hostname) {
                return types::ok(&servers_[i]);
            }
        }
    }
    if (!servers_.empty()) {
        return types::ok(&servers_[0]);
    }
    return types::err(error::kBadRequest);
}

}  // namespace config
}  // namespace http
