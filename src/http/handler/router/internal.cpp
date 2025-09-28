#include "http/request/request.hpp"
#include "http/handler/router/internal.hpp"
#include "http/response/builder.hpp"
#include "utils/logger.hpp" // ログ用追加
#include <sstream> // ← 追加

namespace http {
    InternalRouter::InternalRouter(const RouteRegistry& registry) 
        : registry_(registry) {}
    
    Either<IAction*, Response> InternalRouter::serve(const Request& req) {
        std::ostringstream oss;
        oss << "InternalRouter::serve: called, target=" << req.getRequestTarget()
            << ", method=" << static_cast<int>(req.getMethod());
        LOG_DEBUG(oss.str());

        const types::Option<std::string> matchResult = registry_.matchPath(req.getRequestTarget());
        if (matchResult.isNone()) {
            LOG_DEBUG("InternalRouter::serve: path not found, returning 404");
            return Right(ResponseBuilder().status(kStatusNotFound).build());
        }
        
        const std::string& matchedPath = matchResult.unwrap();
        IHandler* handler = registry_.findHandler(req.getMethod(), matchedPath);
        
        if (!handler) {
            const std::vector<HttpMethod> allowedMethods = registry_.getAllowedMethods(matchedPath);
            if (!allowedMethods.empty()) {
                LOG_DEBUG("InternalRouter::serve: method not allowed, returning 405");
                return Right(ResponseBuilder().status(kStatusMethodNotAllowed).build());
            }
            LOG_DEBUG("InternalRouter::serve: no handler, returning 404");
            return Right(ResponseBuilder().status(kStatusNotFound).build());
        }
        LOG_DEBUG("InternalRouter::serve: handler found, serving request");
        return handler->serve(req);
    }
    
    Matcher<std::string> InternalRouter::createMatcher() const {
        std::map<std::string, std::string> paths;
        const RouteRegistry::HandlerMap& handlers = registry_.getHandlers();
        for (RouteRegistry::HandlerMap::const_iterator it = handlers.begin(); 
             it != handlers.end(); ++it) {
            paths[it->first] = it->first;
        }
        return Matcher<std::string>(paths);
    }
} //namespace http
