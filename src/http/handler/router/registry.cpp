#include "registry.hpp"
#include <set>
namespace http {
    RouteRegistry::RouteRegistry() : pathMatcher_(NULL), matcherDirty_(true) {}
    
    RouteRegistry::~RouteRegistry() {
        std::set<IHandler *> deleted;
        delete pathMatcher_;
        for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
            for (MethodHandlerMap::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
                if (deleted.count(jt->second) == 0) {
                    delete jt->second;
                    deleted.insert(jt->second);
                }
            }
        }
    }
    
    void RouteRegistry::addRoute(HttpMethod method, const std::string& path, IHandler* handler) {
        if (handlers_[path][method]) {
            delete handlers_[path][method];
        }
        handlers_[path][method] = handler;
        invalidateMatcher();
    }
    
    void RouteRegistry::addRoute(const std::vector<HttpMethod>& methods, const std::string& path, IHandler* handler) {
        for (std::vector<HttpMethod>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
            this->addRoute(*it, path, handler);
        }
    }
   
    void RouteRegistry::invalidateMatcher() const {
        matcherDirty_ = true;
    }

    void RouteRegistry::ensureMatcherUpdated() const {
        if (matcherDirty_) {
            delete pathMatcher_;
            
            std::map<Path, Path> paths;
            for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
                paths[it->first] = it->first;
            }
            pathMatcher_ = new Matcher<Path>(paths);
            matcherDirty_ = false;
        }
    }

    types::Option<RouteRegistry::Path> RouteRegistry::matchPath(const std::string& requestPath) const {
        ensureMatcherUpdated();
        return pathMatcher_->match(requestPath);
    }
    IHandler* RouteRegistry::findHandler(HttpMethod method, const std::string& path) const {
        const HandlerMap::const_iterator pathIt = handlers_.find(path);
        if (pathIt != handlers_.end()) {
            const MethodHandlerMap::const_iterator methodIt = pathIt->second.find(method);
            if (methodIt != pathIt->second.end()) {
                return methodIt->second;
            }
        }
        return NULL;
    }
    
    std::vector<HttpMethod> RouteRegistry::getAllowedMethods(const std::string& path) const {
        std::vector<HttpMethod> methods;
        const HandlerMap::const_iterator pathIt = handlers_.find(path);
        if (pathIt != handlers_.end()) {
            for (MethodHandlerMap::const_iterator it = pathIt->second.begin(); 
                 it != pathIt->second.end(); ++it) {
                methods.push_back(it->first);
            }
        }
        return methods;
    }
    
    const RouteRegistry::HandlerMap& RouteRegistry::getHandlers() const {
        return handlers_;
    }
} //namespace http
