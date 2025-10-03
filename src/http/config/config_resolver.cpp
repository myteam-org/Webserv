#include "http/config/config_resolver.hpp"
#include "utils/logger.hpp"

namespace http {
namespace config {

// Hostヘッダー判定はserver_namesだけで行う。host_はバインド用のみ。
ConfigResolver::ConfigResolver(const std::vector<ServerContext>& servers)
    : servers_(servers) {}

types::Result<const ServerContext*, error::AppError>
ConfigResolver::chooseServer(const std::string& host) const {
    // host: "webserv.test:8080" → "webserv.test"
    std::size_t colon = host.find(':');
    std::string hostname = (colon != std::string::npos) ? host.substr(0, colon) : host;

    // server_namesのみで判定。host_は使わない。
    for (std::size_t i = 0; i < servers_.size(); ++i) {
        const std::vector<std::string>& serverNames = servers_[i].getServerNames();
        for (std::size_t j = 0; j < serverNames.size(); ++j) {
            if (serverNames[j] == hostname) {
                return types::ok(&servers_[i]);
            }
        }
    }
    // ここで一致しなかった場合、最初のサーバーブロックを返す（default_server的挙動）
    if (!servers_.empty()) {
        return types::ok(&servers_[0]);
    }
    return types::err(error::kBadRequest);
}

}  // namespace config
}  // namespace http
