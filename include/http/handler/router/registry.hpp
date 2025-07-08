#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "method.hpp"

namespace http {
    // RouteRegistry の修正版
    class RouteRegistry : NonCopyable {
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
        
        // Matcherを使ったパスマッチング
        Option<Path> matchPath(const std::string& requestPath) const;
        
    private:
        HandlerMap handlers_;
        mutable Matcher<Path>* pathMatcher_;  // キャッシュされたMatcher
        mutable bool matcherDirty_;           // Matcherの再生成が必要かどうか
        
        void invalidateMatcher() const;
        void ensureMatcherUpdated() const;
    };
}

