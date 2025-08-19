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

}  // namespace http
