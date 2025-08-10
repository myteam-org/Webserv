#include "cgi.hpp"
#include "config/context/documentRootConfig.hpp"
#include "config/context/serverContext.hpp"
#include "http/request/request.hpp"
#include "utils/string.hpp"

namespace http {
CgiHandler::CgiHandler(const DocumentRootConfig& docRootConfig)
    : docRootConfig_(docRootConfig) {}

Either<IAction*, Response> CgiHandler::serve(const Request& request) {}

Response CgiHandler::serveInternal(const Request& req) const {

}

bool CgiHandler::isCgiTarget(const Request& req, std::string* scriptPath,
                             std::string* pathInfo) const {

}

bool CgiHandler::buildCgiEnv(const Request& req, const std::string& scriptName,
                             const std::string* pathInfo,
                             std::vector<std::string>* env) const {
    if (env == 0) {
        return false;
    }
    env->clear();
    env->push_back("GATEWAY_INTERFACE=CGI/1.1");
    env->push_back("REQUEST_METHOD=" + http::httpMethodToString(req.getMethod()));
    env->push_back("SERVER_PROTOCOL=" + req.getHttpVersion());
    env->push_back("SCRIPT_NAME=" + scriptName);
    env->push_back("SERVER_NAME=" + req.getServer()->getHost());
    env->push_back("SERVER_PORT=" + std::to_string(req.getServer()->getListen()));
    if (!req.getBody().empty()) {
        types::Option<std::string> clOpt = req.getHeader("Content-Length");
        if (clOpt.isSome()) {
            const std::string cLength = clOpt.unwrap();
            env->push_back("CONTENT_LENGTH=" + cLength);
        } else {
            env->push_back("CONTENT_LENGTH=" + std::to_string(req.getBody().size()));
        }
        const types::Option<std::string> contentOpt = req.getHeader("Content-Type");
        if (contentOpt.isSome()) {
            env->push_back("CONTENT_TYPE=" + contentOpt.unwrap());
        }
    }
    if (pathInfo != 0 && !pathInfo->empty()) {
        env->push_back("PATH_INFO=" + *pathInfo);
        env->push_back("PATH_TRANSLATED=" + utils::joinPath(docRootConfig_.getRoot(), *pathInfo));
    }
    env->push_back("QUERY_STRING=" + req.getQueryString());
    env->push_back("DOCUMENT_ROOT=" + docRootConfig_.getRoot());
    return true;
}

bool CgiHandler::runCgi(const std::vector<std::string>& argv,
                        const std::vector<std::string>& env,
                        const std::vector<char>& stdinBody,
                        std::string* stdoutBuf, int* exitCode) const {

}

Response CgiHandler::parseCgiAndBuildResponse(const std::string& cgiOut) const {

}

}  // namespace http
