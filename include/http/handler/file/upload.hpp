#pragma once

#include "config/config.hpp"
#include "either.hpp"
#include "handler.hpp"
#include "http/response/response.hpp"
#include "result.hpp"

namespace http {

class UploadFileHandler : public IHandler {
   public:
    explicit UploadFileHandler(const DocumentRootConfig& docRootConfig);
    Either<IAction*, Response> serve(const Request& request);

   private:
    DocumentRootConfig docRootConfig_;
    Response serveInternal(const Request& request) const;
    static bool isValidUserPath(const std::string& path);
    static bool isPathUnderRoot(const std::string& path,
                                const std::string& root);
    static Response writeToFile(const std::string& path,
                                const std::vector<char>& body);
};
}  // namespace http