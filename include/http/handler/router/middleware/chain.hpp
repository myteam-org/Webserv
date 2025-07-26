#pragma once

#include <vector>
#include "handler/handler.hpp"
#include "middleware/middleware.hpp"

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
            Either<IAction*, Response> serve(const Request& req);
            
        private:
            IMiddleware& middleware_;
            IHandler& next_;
        };
        
        typedef std::vector<IMiddleware*> MiddlewareList;
        MiddlewareList middlewares_;
        std::vector<IHandler*> chainHandlers_;
    };
} //namespace http
