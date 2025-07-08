#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "method.hpp"
#include "option.hpp"
#include "matcher.hpp"

namespace http {
    class RouteRegistry {
    public:
        typedef std::string Path;
        typedef std::map<HttpMethod, IHandler *> MethodHandlerMap;
        typedef std::map<Path, MethodHandlerMap> HandlerMap;

        RouteRegistry();
        ~RouteRegistry();

        void addRoute(HttpMethod method, const std::string& path, IHandler* handler);
        void addRoute(const std::vector<HttpMethod>& methods, const std::string& path, IHandler* handler);

        IHandler* findHandler(HttpMethod method, const std::string& path) const;
        std::vector<HttpMethod> getAllowedMethods(const std::string& path) const;

        const HandlerMap& getHandlers() const;

        types::Option<Path> matchPath(const std::string& requestPath) const;

    private:
        HandlerMap handlers_;
        mutable Matcher<Path>* pathMatcher_;
        mutable bool matcherDirty_;

        void invalidateMatcher() const;
        void ensureMatcherUpdated() const;
    };
} //namespace http
