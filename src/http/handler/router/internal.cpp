#include "internal.hpp"
namespace http {
    InternalRouter::InternalRouter(const RouteRegistry& registry) 
        : registry_(registry) {}
    
    Either<IAction*, Response> InternalRouter::serve(const Request& req) {

        const types::Option<std::string> matchResult = registry_.matchPath(req.getRequestTarget());
        if (matchResult.isNone()) {
            // return Right(ResponseBuilder().status(kStatusNotFound).build());
        }
        
        const std::string& matchedPath = matchResult.unwrap();
        IHandler* handler = registry_.findHandler(req.getMethod(), matchedPath);
        
        if (!handler) {
            const std::vector<HttpMethod> allowedMethods = registry_.getAllowedMethods(matchedPath);
            if (!allowedMethods.empty()) {
                // return Right(ResponseBuilder().status(kStatusMethodNotAllowed).build());
            }
            // return Right(ResponseBuilder().status(kStatusNotFound).build());
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
