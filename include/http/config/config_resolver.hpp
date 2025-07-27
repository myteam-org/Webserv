#pragma once

#include "config/context/serverContext.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {
namespace config {

class IConfigResolver {
   public:
    virtual types::Result<const ServerContext*, error::AppError> choseServer(
        const std::string& host) const = 0;
    virtual ~IConfigResolver() {}
};

class ConfigResolver : public IConfigResolver {
   public:
    explicit ConfigResolver(const std::vector<ServerContext>& servers)
        : servers_(servers) {}

    types::Result<const ServerContext*, error::AppError> choseServer(
        const std::string& host) const {
        const std::size_t colon = host.find(':');
        const std::string& hostname =
            (colon != std::string::npos) ? host.substr(0, colon) : host;
        for (std::size_t i = 0; i < servers_.size(); ++i) {
            if (servers_[i].getHost() == hostname) {
                return types::ok(&servers_[i]);
            }
        }
        return types::err(error::kBadRequest);
    }

    // types::Result<const LocationContext*, error::AppError> choseLocation(
    //     const ServerContext& server, std::string& uri) const {
    //     const std::vector<LocationContext>& locations = server.getLocation();
    //     const LocationContext* bestMatch = NULL;
    //     std::size_t longest = 0;

    //     for (std::size_t i = 0; i < locations.size(); ++i) {
    //         const std::string& path = locations[i].getPath();
    //         if (uri.compare(0, path.size(), path) == 0) {
    //             if (path.size() > longest) {
    //                 longest = path.size();
    //                 bestMatch = &locations[i];
    //             }
    //             return types::ok(&locations[i]);
    //         }
    //     }
    //     if (bestMatch) {
    //         return types::ok(bestMatch);
    //     }
    //     return types::err(error::kBadRequest);
    // }

   private:
    std::vector<ServerContext> servers_;
};
}  // namespace config
}  // namespace http