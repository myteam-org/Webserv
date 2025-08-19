#include "cgi.hpp"

#include "utils/string.hpp"
#include "utils/path.hpp"
#include "http/request/request.hpp"
#include "config/context/serverContext.hpp"

namespace http {

namespace {
inline void addEnv(std::vector<std::string>* env,
                          const char* key,
                          const std::string& value) {
    env->push_back(std::string(key) + "=" + value);
}
}

bool CgiHandler::buildCgiEnv(const http::Request& req,
                             const std::string& scriptName,
                             const std::string* pathInfo,
                             std::vector<std::string>* env) const {
    if (env == NULL) {
        return false;
    }
    env->clear();
    const ServerContext* server = req.getServer();
    addEnv(env, "GATEWAY_INTERFACE", "CGI/1.1");
    addEnv(env, "REQUEST_METHOD", http::httpMethodToString(req.getMethod()));
    addEnv(env, "SERVER_PROTOCOL", req.getHttpVersion());
    addEnv(env, "SCRIPT_NAME", scriptName);
    if (server != NULL) {
        addEnv(env, "SERVER_NAME", server->getHost());
        addEnv(env, "SERVER_PORT", utils::toString(server->getListen()));
    }
    if (pathInfo != NULL && !pathInfo->empty()) {
        addEnv(env, "PATH_INFO", *pathInfo);
        addEnv(env, "PATH_TRANSLATED", utils::joinPath(docRootConfig_.getRoot(), *pathInfo));
    }
    addEnv(env, "QUERY_STRING", req.getQueryString());
    addEnv(env, "DOCUMENT_ROOT", docRootConfig_.getRoot());
    const types::Option<std::string> clOpt = req.getHeader("Content-Length");
    if (clOpt.isSome()) {
        addEnv(env, "CONTENT_LENGTH", clOpt.unwrap());
    } else if (!req.getBody().empty()) {
        addEnv(env, "CONTENT_LENGTH", utils::toString(req.getBody().size()));
    }
    const types::Option<std::string> contentOpt = req.getHeader("Content-Type");
    if (contentOpt.isSome()) {
        addEnv(env, "CONTENT_TYPE", contentOpt.unwrap());
    }
    return true;
}

} // namespace http
