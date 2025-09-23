#pragma once

#include "config/config.hpp"
#include "utils/types/either.hpp"
#include "http/handler/handler.hpp"
#include "http/response/response.hpp"
#include "utils/types/result.hpp"

static const unsigned char kAsciiSpace = 0x20;

namespace http {

class UploadFileHandler : public IHandler {
   public:
    explicit UploadFileHandler(const DocumentRootConfig& docRootConfig);
    Either<IAction*, Response> serve(const Request& request);

   private:
    DocumentRootConfig docRootConfig_;
    Response serveInternal(const Request& request) const;
    bool decodeAndNomalizePath(const std::string& rawPath,
                               std::string& normalized) const;
    static bool urlDecodeStrict(const std::string& src, std::string& out);
    types::Result<types::Unit, HttpStatusCode> checkParentDir(
        const std::string& normalized) const;
    static Response writeToFile(const std::string& path,
                                const std::vector<char>& body);
};
}  // namespace http
