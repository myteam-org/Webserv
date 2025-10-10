#include "http/handler/router/builder.hpp"
#include "http/handler/router/router.hpp"
#include "http/handler/router/registry.hpp"
#include "http/handler/router/middleware/chain.hpp"
#include "utils/logger.hpp"

namespace http {
    RouterBuilder::RouterBuilder()
        : routerInstance_(new Router()), isBuilt_(false) {}

    RouterBuilder::~RouterBuilder() {
        if (!isBuilt_ && routerInstance_ != NULL) {
            delete routerInstance_;
        }
    }

    RouterBuilder& RouterBuilder::route(const HttpMethod httpMethod, const std::string& routePath, IHandler* routeHandler) {
        if (isBuilt_) {
            LOG_ERROR("RouterBuilder: Cannot add route after build()");
            return *this;
        }
        routerInstance_->routeRegistry_->addRoute(httpMethod, routePath, routeHandler);
        return *this;
    }

    RouterBuilder& RouterBuilder::route(const std::vector<HttpMethod>& httpMethodList,
                                       const std::string& routePath, IHandler* routeHandler) {
        if (isBuilt_) {
            LOG_ERROR("RouterBuilder: Cannot add route after build()");
            return *this;
        }
        routerInstance_->routeRegistry_->addRoute(httpMethodList, routePath, routeHandler);
        return *this;
    }

    RouterBuilder& RouterBuilder::routeForExtension(HttpMethod method, const std::string& path,
                                                    const std::string& ext, IHandler* handler) {
        std::cerr << "routeForExtension" << std::endl;
        routerInstance_->routeRegistry_->addRouteForExtension(method, path, ext, handler);
        return *this;
    }

    RouterBuilder& RouterBuilder::middleware(IMiddleware* middlewareInstance) {
        if (isBuilt_) {
            LOG_ERROR("RouterBuilder: Cannot add middleware after build()");
            return *this;
        }
        routerInstance_->middlewareChain_->addMiddleware(middlewareInstance);
        return *this;
    }

    Router* RouterBuilder::build() {
        if (isBuilt_) {
            LOG_ERROR("RouterBuilder: build() called multiple times");
            return NULL;
        }
        isBuilt_ = true;
        routerInstance_->compile();
        Router* resultRouter = routerInstance_;
        routerInstance_ = NULL;  // Release ownership
        return resultRouter;
    }
} // namespace http
