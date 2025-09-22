#pragma once

#include "utils/types/either.hpp"
#include "http/handler/handler.hpp"
#include "http/handler/router/middleware/middleware.hpp"
#include "config/config.hpp"

namespace http {

class ErrorPage : public IMiddleware {
 public:
    explicit ErrorPage(const ErrorPageMap& errorPageMap);

    virtual Either<IAction*, Response> intercept(
        const Request& request,
        IHandler& nextHandler);

 private:
    const ErrorPageMap errorPageMap_;
};

}  // namespace http
