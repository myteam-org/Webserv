#include "http/handler/router/registry.hpp"
#include "utils/types/option.hpp"
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
    
    // この path で、拡張子が ext のとき、method は handler を使う
    // 例: ("/", ".py", GET) -> CgiHandler* の設定で、「/hello.py に GET が来たら CGI」を選べるようになる
    void RouteRegistry::addRouteForExtension(HttpMethod method, const std::string& path, const std::string& ext, IHandler* handler) {
        // 例: path="/", ext=".py"
        IHandler*& slot = extHandlers_[path][ext][method];
        if (slot) {
            delete slot;
        }
        slot = handler;
        invalidateMatcher();
    }

    // 同じ path に対しても、拡張子（例: .py）で優先ハンドラを切り替える
    // データ構造（イメージ）
    // handlers_      : path -> (method -> handler)            既存：拡張子に関係ないデフォルト
    // extHandlers_   : path -> (ext -> (method -> handler))   追加
    IHandler* RouteRegistry::findHandlerFor(HttpMethod method, const std::string& path, const std::string& ext /* ".py" or "" */) const {
        // 拡張子優先
        if (!ext.empty()) {
            ExtHandlerMap::const_iterator pit = extHandlers_.find(path);
            if (pit != extHandlers_.end()) {
                ExtMethodMap::const_iterator eit = pit->second.find(ext);
                if (eit != pit->second.end()) {
                    MethodHandlerMap::const_iterator mit = eit->second.find(method);
                    if (mit != eit->second.end()) {
                        return mit->second;
                    }
                }
            }
        }
        // なければ従来どおり
        return findHandler(method, path);
    }

    void RouteRegistry::invalidateMatcher() const {
        matcherDirty_ = true;
    }

    // マッチャには “/ で始まるキーのみ” を入れる
    void RouteRegistry::ensureMatcherUpdated() const {
        if (matcherDirty_) {
            delete pathMatcher_;
            pathMatcher_ = NULL;
            
            std::map<Path, Path> paths;
            for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
                const std::string& key = it->first;
                if (!key.empty() && key[0] == '/') {
                    paths[key] = key;
                }
                
            }
            pathMatcher_ = new Matcher<Path>(paths);
            matcherDirty_ = false;
        }
    }

    // 前方一致 → ダメなら拡張子末尾一致
    types::Option<RouteRegistry::Path> RouteRegistry::matchPath(const std::string& requestPath) const {
        ensureMatcherUpdated();
    
        // クエリを落とした「パスのみ」
        const std::string::size_type query = requestPath.find('?');
        const std::string pathOnly = (query == std::string::npos) ? requestPath : requestPath.substr(0, query);
        // 通常の前方一致
        types::Option<Path> prefix = pathMatcher_->match(requestPath);
        // ".ext" キーで末尾一致（最長を優先：.tar.gz > .gz）
        std::string bestKey;
        std::size_t bestLen = 0;
        for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
            const std::string& key = it->first;
            if (key.empty() || key[0] != '.') continue;  // ドットで始まるキーだけ対象
            if (pathOnly.size() >= key.size() &&
                pathOnly.compare(pathOnly.size() - key.size(), key.size(), key) == 0) {
                if (key.size() > bestLen) { bestKey = key; bestLen = key.size(); }
            }
        }
        if (bestLen > 0) {
            return types::some(bestKey);
        }
        if (!prefix.isNone()) {
            return prefix;
        }
        // どれにも合わなければ None
        return types::none<Path>();
        // return pathMatcher_->match(requestPath);
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
