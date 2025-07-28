#pragma once

#include "utils/types/result.hpp"
#include "utils/types/error.hpp"
#include "raw_headers.hpp"

class ReadContext;
class RequestParser;
class ReadBuffer;
class Request;

namespace http {
namespace parse {

    class RequestParser {
       public:
        explicit RequestParser(const ReadContext& ctx);
        ~RequestParser();
    
        types::Result<Request, error::AppError> parseRequest();

       private:
        const ReadContext& ctx_;
        std::string method_;
        std::string uri_;
        std::string version_;

        RawHeaders headers_;

        std::vector<char> body_;

        const LocationContext* location_;
        const ServerContext* server_;
        const DocumentRootConfig* documentRoot_;

        types::Result<types::Unit, error::AppError> parseRequestLine();
        types::Result<types::Unit, error::AppError> parseHeaders();
        bool checkMissingHost() const;
        bool validateContentLength() const;
        bool containNonDigit(const std::string& val) const;
        bool validateTransferEncoding() const;
        types::Result<types::Unit, error::AppError> parseBody();
        Request buildRequest() const;
        types::Result<const LocationContext*, error::AppError> choseLocation(std::string& uri);
    };

} // namespace parse
} // namespace http

