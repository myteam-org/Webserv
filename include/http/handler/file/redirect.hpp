#pragma once

#include "http/handler/handler.hpp"

namespace http {
    class RedirectHandler : public IHandler {
    public:
        explicit RedirectHandler(const std::string &destination);
        Either<IAction *, Response> serve(const Request &request);

    private:
        std::string destination_;

        Response serveInternal(const Request &req) const;
    };
} //namespace http

