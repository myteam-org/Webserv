#pragma once

#include "http/handler/router/internal.hpp"
#include "http/handler/router/middleware/chain.hpp"

namespace http {
    class Router : public IHandler {
    public:
        Router();
        ~Router();

        Either<IAction*, Response> serve(const Request& request);

    private:
        friend class RouterBuilder;

        RouteRegistry* routeRegistry_;
        MiddlewareChain* middlewareChain_;
        InternalRouter* internalRouter_;
        IHandler* compiledHandler_;

        void compile();
    };
} //namespace http
