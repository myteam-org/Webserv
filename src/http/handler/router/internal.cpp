#include <sstream>
#include "http/request/request.hpp"
#include "http/handler/router/internal.hpp"
#include "http/response/builder.hpp"
#include "http/method.hpp" // isMethodImplementedを使う
#include "utils/logger.hpp"
#include "utils/path.hpp"

namespace http {
    InternalRouter::InternalRouter(const RouteRegistry& registry) 
        : registry_(registry) {}

    Either<IAction*, Response> InternalRouter::serve(const Request& req) {
        // GET/POST/DELETE以外は501
        if (!isMethodImplemented(req.getMethod())) {
            return Right(ResponseBuilder().status(kStatusNotImplemented).build());
        }
        // パスのみでマッチ
        const std::string& target = req.getRequestTarget();
        const std::string::size_type query = target.find('?');
        const std::string pathOnly = (query == std::string::npos) ? target : target.substr(0, query);

        const types::Option<std::string> matchResult = registry_.matchPath(pathOnly);
        if (matchResult.isNone()) {
            return Right(ResponseBuilder().status(kStatusNotFound).build());
        }

        const std::string& matchedPath = matchResult.unwrap(); // "/" や "/upload" や ".py"
        LOG_INFO("MatchedPath information:" + matchedPath + "");

        // IHandler* handler = registry_.findHandler(req.getMethod(), matchedPath);
        IHandler* handler = registry_.findHandler(req.getMethod(), matchedPath);

        if (!handler) {
            LOG_WARN("handler is not registered. matched path or method is not allowed");
            const std::vector<HttpMethod> allowedMethods = registry_.getAllowedMethods(matchedPath);
            if (!allowedMethods.empty()) {
                return Right(ResponseBuilder().status(kStatusMethodNotAllowed).build());
            }
            return Right(ResponseBuilder().status(kStatusNotFound).build());
        }
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
