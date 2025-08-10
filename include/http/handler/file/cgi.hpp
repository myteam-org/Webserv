#pragma once

#include "handler/handler.hpp"

namespace http {
class CgiHandler : public IHandler {
   public:
    explicit CgiHandler(const DocumentRootConfig &docRootConfig);
    Either<IAction *, Response> serve(const Request &request);
};
} // namespace http
