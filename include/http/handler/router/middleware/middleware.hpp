#pragma once
#include "action.hpp"
#include "handler.hpp"

namespace http {
    class IMiddleware {
    public:
        virtual ~IMiddleware() {}
        virtual Either<IAction *, Response> intercept(const Request &ctx, IHandler &next) = 0;
    };
} //namespace 
