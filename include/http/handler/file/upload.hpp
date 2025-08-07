#pragma once

#include "handler.hpp"
#include "config/config.hpp"
#include "result.hpp"
#include "http/response/response.hpp"

namespace http {

class UploadFileHandler : public IHandler {
   public:
    explicit UploadFileHandler(const DocumentRootConfig& docRootConfig);
    Either<IAction*, Response> serve(const Request& req);
   private:
    DocumentRootConfig docRootConfig_;
    Response serveInternal(const Request& req) const;
};
} // namespace http