#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "http/method.hpp"
#include "utils/types/option.hpp"
#include "http/handler/matcher.hpp"
#include "http/handler/handler.hpp"

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
        // 拡張子用の登録（例: ".py"）
        void addRouteForExtension(HttpMethod method, const std::string& path, const std::string& ext, IHandler* handler);

        IHandler* findHandler(HttpMethod method, const std::string& path) const;
        // 既存の findHandler に “拡張子考慮版” を足す
        IHandler* findHandlerFor(HttpMethod method, const std::string& path, const std::string& ext /* ".py" or "" */) const;

        std::vector<HttpMethod> getAllowedMethods(const std::string& path) const;

        const HandlerMap& getHandlers() const;

        types::Option<Path> matchPath(const std::string& requestPath) const;

    private:
        HandlerMap handlers_;

        // ★追加: path -> (ext -> (method -> handler))
        typedef std::map<std::string, MethodHandlerMap> ExtMethodMap;
        typedef std::map<Path, ExtMethodMap>            ExtHandlerMap;
        ExtHandlerMap extHandlers_;

        mutable Matcher<Path>* pathMatcher_;
        mutable bool matcherDirty_;

        void invalidateMatcher() const;
        void ensureMatcherUpdated() const;
    };
} //namespace http

// 登録されたルートの一覧保持
