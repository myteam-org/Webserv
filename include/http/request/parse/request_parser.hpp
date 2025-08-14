#pragma once

#include "config/context/locationContext.hpp"
#include "config/context/serverContext.hpp"
#include "context.hpp"
#include "http/request/request.hpp"
#include "raw_headers.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"

namespace http {
class ReadContext;
}

namespace http {
namespace parse {

class RequestParser {
   public:
    explicit RequestParser(http::ReadContext& ctx);
    ~RequestParser();

    types::Result<types::Unit, error::AppError> parseRequestLine();
    types::Result<types::Unit, error::AppError> parseHeaders();
    types::Result<types::Unit, error::AppError> parseBody();
    types::Result<const LocationContext*, error::AppError> chooseLocation(
        const std::string& uri) const;

   private:
    ReadContext* ctx_;
    HttpMethod method_;
    std::string requestTarget_;
    std::string pathOnly_;
    std::string queryString_;
    std::string version_;

    RawHeaders headers_;

    std::vector<char> body_;

    bool checkMissingHost() const;
    bool validateContentLength() const;
    bool validateTransferEncoding() const;
    types::Result<types::Unit, error::AppError> decodeAndNormalizeTarget(
        const std::string& requestTarget);
    types::Result<Request, error::AppError> buildRequest() const;
};

}  // namespace parse
}  // namespace http
