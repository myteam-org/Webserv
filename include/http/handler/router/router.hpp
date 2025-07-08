#pragma once

#include "http/handler/router/registry.hpp"
#include "http/handler/router/middleware/chain.hpp"
#include "http/handler/router/internal.hpp"

namespace http {
    class Router : public IHandler {
    public:
        Router();
        ~Router();

        // Either<IAction*, Response> serve(const RequestContext& ctx);

    private:
        friend class RouterBuilder;

        RouteRegistry* routeRegistry_;
        MiddlewareChain* middlewareChain_;
        InternalRouter* internalRouter_;
        IHandler* compiledHandler_;

        void compile();
    };
} //namespace http
