#pragma once

#include "raw_headers.hpp"
#include "context.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include "config/context/serverContext.hpp"
#include "config/context/locationContext.hpp"
#include "config/context/documentRootConfig.hpp"

namespace http {
    class ReadContext;
    class HttpRequest;
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
    const std::string& getMethod() const;
    const std::string& getRequestTarget() const;
    const std::string& getPath() const;
    const std::string& getQueryString() const;
    const std::string& getVersion() const;
    const RawHeaders& getHeaders() const;
    const std::vector<char>& getBody()const;

   private:
    ReadContext* ctx_;
    std::string method_;
    std::string uri_;
    std::string version_;

    RawHeaders headers_;

    std::vector<char> body_;

    const LocationContext* location_;
    const ServerContext* server_;
    // const DocumentRootConfig* documentRoot_;

    bool checkMissingHost() const;
    bool validateContentLength() const;
    bool validateTransferEncoding() const;
    types::Result<HttpRequest, error::AppError> buildRequest() const;
};

}  // namespace parse
}  // namespace http
