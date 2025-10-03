#pragma once

#include "config/context/serverContext.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {
namespace config {

// Hostヘッダー判定はserver_namesだけで行う。host_はバインド用のみ。
class IConfigResolver {
   public:
    virtual types::Result<const ServerContext*, error::AppError> chooseServer(
        const std::string& host) const = 0;
    virtual ~IConfigResolver() {}
};

class ConfigResolver : public IConfigResolver {
   public:
    explicit ConfigResolver(const std::vector<ServerContext>& servers);

    types::Result<const ServerContext*, error::AppError> chooseServer(
        const std::string& host) const;

   private:
    std::vector<ServerContext> servers_;
};

}  // namespace config
}  // namespace http
