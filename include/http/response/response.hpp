#pragma once


#include "http/status.hpp"
#include "utils/types/option.hpp"
#include "http/response/response_header_types.hpp"
#include <iostream>

namespace http {
    class Response {
    public:
        explicit Response(
            HttpStatusCode status,
            const types::Option<std::string> &body = types::none<std::string>(),
            const std::string &httpVersion = defaultHttpVersion
        );
        // ヘッダー付きコンストラクタ
        Response(HttpStatusCode status,
                const ResponseHeaderFields& headers,
                const types::Option<std::string>& body,
                const std::string& httpVersion);

        bool operator==(const Response &other) const;

        HttpStatusCode getStatusCode() const;
        const ResponseHeaderFields &getHeaders() const;
        const std::string &getHttpVersion() const;
        const types::Option<std::string> &getBody() const;
        std::string toString();
        
    private:
        static const std::string defaultHttpVersion;
        HttpStatusCode status_;
        ResponseHeaderFields headers_;
        types::Option<std::string> body_;
        std::string httpVersion_;
    };
} //namespace http
