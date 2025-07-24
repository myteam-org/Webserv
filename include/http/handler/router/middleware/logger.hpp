#pragma once

#include "./middleware.hpp"
#include "utils/logger.hpp"
#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "utils/types/either.hpp"

namespace http {

class Logger : public IMiddleware {
public:
    Logger() {}
    virtual ~Logger() {}

    virtual Either<IAction*, Response> intercept(const Request &requestContext, IHandler &nextHandler);
};

}  // namespace http
