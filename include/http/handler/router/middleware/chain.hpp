#pragma once
#include <vector>

namespace http {
    class MiddlewareChain {
    public:
        MiddlewareChain();
        ~MiddlewareChain();
        
        void addMiddleware(IMiddleware* middleware);
        IHandler* buildChain(IHandler* finalHandler);
        
    private:
        class ChainedHandler : public IHandler {
        public:
            ChainedHandler(IMiddleware& middleware, IHandler& next);
            Either<IAction*, Response> serve(const RequestContext& ctx);
            
        private:
            IMiddleware& middleware_;
            IHandler& next_;
        };
        
        typedef std::vector<IMiddleware*> MiddlewareList;
        MiddlewareList middlewares_;
        std::vector<IHandler*> chainHandlers_;
    };
} //namespace http
