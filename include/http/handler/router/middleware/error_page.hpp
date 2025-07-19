#pragma once

#include "utils/types/either.hpp"
#include "handler/handler.hpp"
#include "config/config.hpp"

namespace http {

class ErrorPage : public IHandler {
 public:
    explicit ErrorPage(const ErrorPageMap& errorPageMap);

    virtual Either<IAction*, Response> intercept(
        const Request& request,
        IHandler& nextHandler);

 private:
    const ErrorPageMap errorPageMap_;
};

}  // namespace http
