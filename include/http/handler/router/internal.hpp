#pragma once
#include "http/request/request.hpp"
#include "http/handler/handler.hpp"
#include "http/handler/router/registry.hpp"

namespace http {
    class InternalRouter : public IHandler {
    public:
        explicit InternalRouter(const RouteRegistry& registry);
        Either<IAction*, Response> serve(const Request& req);
        
    private:
        const RouteRegistry& registry_;
        Matcher<std::string> createMatcher() const;
    };
} // namespace http

// 実際に Method + Path を照合してハンドラを決める処理
