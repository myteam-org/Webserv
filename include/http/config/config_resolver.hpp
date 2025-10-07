#pragma once

#include <vector> // ← これが必須！
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
    explicit ConfigResolver(const std::vector<ServerContext>& servers);
    virtual ~ConfigResolver();
    virtual types::Result<const ServerContext*, error::AppError> chooseServer(
        const std::string& host) const;

private:
    std::vector<ServerContext> servers_;
};

}  // namespace config
}  // namespace http
