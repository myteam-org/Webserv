#include "http/handler/file/cgi.hpp"

#include "config/context/documentRootConfig.hpp"
#include "config/context/serverContext.hpp"
#include "http/request/request.hpp"
#include "http/response/builder.hpp"
#include "utils/string.hpp"

#include "utils/logger.hpp"
#include "action/cgi_action.hpp"

namespace http {
CgiHandler::CgiHandler(const DocumentRootConfig& docRootConfig)
    : docRootConfig_(docRootConfig) {}

Either<IAction*, Response> CgiHandler::serve(const Request& req) {
    std::string scriptPath;
    std::string pathInfo;
    Logger& log = Logger::instance();
    LOG_INFO("cgiHandler serve: function invoked");
    if (!isCgiTarget(req, &scriptPath, &pathInfo)) {
        LOG_WARN("cgiHandler serve: cannot find CgiTarget");
        return Right(ResponseBuilder().status(kStatusNotFound).build());
    }
    return prepareCgi(req);
    // return Right(this->serveInternal(req));
}

Either<IAction*, Response> CgiHandler::prepareCgi(const Request& req) {
    std::string scriptPath;
    std::string pathInfo;
    if (!isCgiTarget(req, &scriptPath, &pathInfo)) {
        return Right(ResponseBuilder().status(kStatusNotFound).build());
    }
    const std::string joined = utils::joinPath(docRootConfig_.getRoot(), scriptPath);
    const std::string realScriptPath = utils::normalizePath(joined);
    std::string rel = req.getPath();
    if (!rel.empty() && rel[0] == '/') {
        rel.erase(0, 1);
    }
    const std::string full = utils::joinPath(realScriptPath, rel);
    LOG_DEBUG("CgiHandler::prepareCgi: joined :" + joined);
    LOG_DEBUG("CgiHandler::prepareCgi: realScriptPath :" + realScriptPath);
    LOG_DEBUG("CgiHandler::prepareCgi: full :" + full);

    std::vector<std::string> env;
    buildCgiEnv(req, scriptPath, &pathInfo, &env);

    std::vector<std::string> argv;
    argv.push_back(full);

    const std::vector<char>& stdinBody = req.getBody();
    PreparedCgi pc;
    pc.argv = argv;
    pc.env  = env;
    pc.stdinBody = &req.getBody();
    pc.owner  = this;
    pc.parseFn = &CgiHandler::parseCgiAndBuildResponse;

    return Left(static_cast<IAction*>(new CgiActionPrepared(pc)));
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
    if (!executeCgi(argv, env, stdinBody, &cgiOut, &exitCode)) {
        return ResponseBuilder().status(kStatusBadGateway).build();
    }

    return parseCgiAndBuildResponse(cgiOut);
}

}  // namespace http
