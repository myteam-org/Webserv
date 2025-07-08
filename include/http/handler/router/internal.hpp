#pragma once
#include <iostream>

namespace http {
    class InternalRouter : public IHandler {
    public:
        explicit InternalRouter(const RouteRegistry& registry);
        Either<IAction*, Response> serve(const RequestContext& ctx);
        
    private:
        const RouteRegistry& registry_;
        Matcher<std::string> createMatcher() const;
    };
} // namespace http
