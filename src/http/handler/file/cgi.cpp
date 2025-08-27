#include "cgi.hpp"

#include "config/context/documentRootConfig.hpp"
#include "config/context/serverContext.hpp"
#include "http/request/request.hpp"
#include "http/response/builder.hpp"
#include "utils/string.hpp"
#include "utils/path.hpp"

namespace http {
CgiHandler::CgiHandler(const DocumentRootConfig& docRootConfig)
    : docRootConfig_(docRootConfig) {}

Either<IAction*, Response> CgiHandler::serve(const Request& req) {
    std::string scriptPath;
    std::string pathInfo;

    if (!isCgiTarget(req, &scriptPath, &pathInfo)) {
        return Right(ResponseBuilder().status(kStatusNotFound).build());
    }
    // 正規化
    const std::string root = docRootConfig_.getRoot();
    const std::string joined = utils::joinPath(root, scriptPath);
    const std::string realScriptPath = utils::path::normalizeSlashes(joined);
    std::vector<std::string> env;
    buildCgiEnv(req, scriptPath, &pathInfo, &env);

    const std::vector<std::string> argv(1, realScriptPath);
    const std::vector<char>& stdinBody = req.getBody();

    IAction* task = createCgiTask(argv, env, stdinBody);
    if (!task) {
        return Right(ResponseBuilder().status(kStatusBadGateway).build());
    }
    return Left(task);
}

}  // namespace http
