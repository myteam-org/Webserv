#include "builder.hpp"

namespace http {
    RouterBuilder::RouterBuilder() 
        : router_(new Router()), built_(false) {}
    
    RouterBuilder::~RouterBuilder() {
        if (!built_ && router_) {
            delete router_;
        }
    }
    
    RouterBuilder& RouterBuilder::route(HttpMethod method, const std::string& path, IHandler* handler) {
        if (built_) {
            // すでにビルド済みの場合はエラー
            LOG_ERROR("RouterBuilder: Cannot add route after build()");
            return *this;
        }
        router_->routeRegistry_->addRoute(method, path, handler);
        return *this;
    }
    
    RouterBuilder& RouterBuilder::route(const std::vector<HttpMethod>& methods, 
                                       const std::string& path, IHandler* handler) {
        if (built_) {
            LOG_ERROR("RouterBuilder: Cannot add route after build()");
            return *this;
        }
        router_->routeRegistry_->addRoute(methods, path, handler);
        return *this;
    }
    
    RouterBuilder& RouterBuilder::middleware(IMiddleware* middleware) {
        if (built_) {
            LOG_ERROR("RouterBuilder: Cannot add middleware after build()");
            return *this;
        }
        router_->middlewareChain_->addMiddleware(middleware);
        return *this;
    }
    
    Router* RouterBuilder::build() {
        if (built_) {
            LOG_ERROR("RouterBuilder: build() called multiple times");
            return NULL;
        }
        built_ = true;
        router_->compile();
        Router* result = router_;
        router_ = NULL;  // 所有権を放棄
        return result;
    }
} //namespace http
