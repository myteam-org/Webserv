#include "http/handler/file/cgi.hpp"

#include "config/context/documentRootConfig.hpp"
#include "config/context/serverContext.hpp"
#include "http/request/request.hpp"
#include "http/response/builder.hpp"
#include "utils/string.hpp"

#include "utils/logger.hpp"
namespace http {
CgiHandler::CgiHandler(const DocumentRootConfig& docRootConfig)
    : docRootConfig_(docRootConfig) {}

Either<IAction*, Response> CgiHandler::serve(const Request& req) {
    std::string scriptPath;
    std::string pathInfo;
    Logger& log = Logger::instance();
    LOG_INFO("cgiHandler serve:");
    if (!isCgiTarget(req, &scriptPath, &pathInfo)) {
        return Right(ResponseBuilder().status(kStatusNotFound).build());
    }
    return Left(static_cast<IAction*>(0));
    // return Right(this->serveInternal(req));
}

Response CgiHandler::serveInternal(const Request& req) const {
    std::string scriptPath;
    std::string pathInfo;
    if (!isCgiTarget(req, &scriptPath, &pathInfo)) {
        return ResponseBuilder().status(kStatusNotFound).build();
    }
    const std::string joined = utils::joinPath(docRootConfig_.getRoot(), scriptPath);
    const std::string realScriptPath = utils::normalizePath(joined);

    std::vector<std::string> env;
    buildCgiEnv(req, scriptPath, &pathInfo, &env);

    std::vector<std::string> argv;
    argv.push_back(realScriptPath);

    const std::vector<char>& stdinBody = req.getBody();

    std::string cgiOut;
    int exitCode = 0;
    // if (!executeCgi(argv, env, stdinBody, &cgiOut, &exitCode)) {
    //     return ResponseBuilder().status(kStatusBadGateway).build();
    // }

    return parseCgiAndBuildResponse(cgiOut);
}

}  // namespace http
