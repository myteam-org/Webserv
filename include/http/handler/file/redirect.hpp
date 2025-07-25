#pragma once

#include "handler.hpp"

namespace http {
    class RedirectHandler : public IHandler {
    public:
        explicit RedirectHandler(const std::string &destination);
        Either<IAction *, Response> serve(const Request &ctx);

    private:
        std::string destination_;

        Response serveInternal(const Request &req) const;
    };
} //namespace http

