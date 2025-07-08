#pragma once
namespace http {
    class Router : NonCopyable, public IHandler {
    public:
        Router();
        ~Router();
        
        Either<IAction*, Response> serve(const RequestContext& ctx);
        
    private:
        friend class RouterBuilder;
        
        RouteRegistry* routeRegistry_;
        MiddlewareChain* middlewareChain_;
        InternalRouter* internalRouter_;
        IHandler* compiledHandler_;
        
        void compile();
    };
} //namespace http
