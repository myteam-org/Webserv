#include "http/handler/router/middleware/logger.hpp"
#include "http/method.hpp"

namespace http {

Either<IAction*, Response> Logger::intercept(const Request &requestContext, IHandler &nextHandler) {
    const std::string requestMethodString = httpMethodToString(requestContext.getMethod());
    const std::string &requestTargetString = requestContext.getRequestTarget();

    LOG_INFO("<-- " + requestMethodString + " " + requestTargetString);

    const Either<IAction*, Response> handlerResult = nextHandler.serve(requestContext);
    if (handlerResult.isLeft()) {
        LOG_INFO("--> " + requestMethodString + " " + requestTargetString + " Pending");
        return handlerResult;
    }

    const Response responseObject = handlerResult.unwrapRight();
    std::ostringstream responseLogStream;
    responseLogStream << "--> " << requestMethodString << " " << requestTargetString << " " << responseObject.getStatusCode();
    LOG_INFO(responseLogStream.str());

    return Right<Response>(responseObject);
}

}  // namespace http
