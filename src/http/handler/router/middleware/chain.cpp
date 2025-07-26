#include "middleware/chain.hpp"
namespace http {
    MiddlewareChain::MiddlewareChain() {}
    
    MiddlewareChain::~MiddlewareChain() {
        for (MiddlewareList::const_iterator it = middlewares_.begin(); it != middlewares_.end(); ++it) {
            delete *it;
        }
        for (std::vector<IHandler*>::iterator it = chainHandlers_.begin(); it != chainHandlers_.end(); ++it) {
            delete *it;
        }
    }
    
    void MiddlewareChain::addMiddleware(IMiddleware* middleware) {
        middlewares_.push_back(middleware);
    }
    
    IHandler* MiddlewareChain::buildChain(IHandler* finalHandler) {
        if (middlewares_.empty()) {
            return finalHandler;
        }
        
        // チェーンを構築（FILO順）
        IHandler* current = finalHandler;
        for (size_t i = middlewares_.size(); i > 0; --i) {
            ChainedHandler* chained = new ChainedHandler(*middlewares_[i-1], *current);
            chainHandlers_.push_back(chained);
            current = chained;
        }
        
        return current;
    }
    
    // MiddlewareChain::ChainedHandler の実装
    MiddlewareChain::ChainedHandler::ChainedHandler(IMiddleware& middleware, IHandler& next)
        : middleware_(middleware), next_(next) {}
    
    Either<IAction*, Response> MiddlewareChain::ChainedHandler::serve(const Request& req) {
        return middleware_.intercept(req, next_);
    }
} //namespace http
