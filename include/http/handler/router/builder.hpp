#pragma once

#include <string>
#include <vector>
#include "http/method.hpp" // HttpMethodのenum定義をインクルード

namespace http {

    class IHandler;
    class IMiddleware;
    class Router;

    class RouterBuilder {
    public:
        RouterBuilder();
        ~RouterBuilder();

        RouterBuilder& route(HttpMethod httpMethod, const std::string& routePath, IHandler* routeHandler);
        RouterBuilder& route(std::vector<HttpMethod>& httpMethodList, const std::string& routePath, IHandler* routeHandler);
        RouterBuilder& middleware(IMiddleware* middlewareInstance);

        Router* build();

    private:
        Router* routerInstance_;
        bool isBuilt_;
    };

} // namespace http
