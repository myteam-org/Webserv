#include <sstream>
#include "http/handler/router/router.hpp"
#include "utils/logger.hpp"

namespace http {Router::Router() 
        : routeRegistry_(new RouteRegistry()),
          middlewareChain_(new MiddlewareChain()),
          internalRouter_(NULL),
          compiledHandler_(NULL) {}
    
    Router::~Router() {
        delete routeRegistry_;
        delete middlewareChain_;
        delete internalRouter_;
    }
    
    Either<IAction*, Response> Router::serve(const Request& req) {
        if (!compiledHandler_) {
            compile();
        }
        return compiledHandler_->serve(req);
    }

    void Router::compile() {
        if (!internalRouter_) {
            internalRouter_ = new InternalRouter(*routeRegistry_);
        }
        compiledHandler_ = middlewareChain_->buildChain(internalRouter_);
    }
} //namespace http
