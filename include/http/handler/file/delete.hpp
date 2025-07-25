#pragma once
#include "handler.hpp"
#include "delete.hpp"
#include "config/config.hpp"

namespace http {
    class DeleteFileHandler : public IHandler {
    public:
        explicit DeleteFileHandler(const DocumentRootConfig &docRootConfig);
        Either<IAction *, Response> serve(const Request &request);

    private:
        DocumentRootConfig docRootConfig_;

        Response serveInternal(const Request &req) const;
    };
} //namespace http
