#pragma once

#include "status.hpp"
#include "http/header.hpp"
#include "option.hpp"
#include <iostream>

namespace http {
    class Response {
    public:
        explicit Response(
            HttpStatusCode status,
            // const Headers &headers = Headers(),
            const types::Option<std::string> &body = types::none<std::string>,
            const std::string &httpVersion = "HTTP/1.1"
        );
        bool operator==(const Response &other) const;

        HttpStatusCode getStatusCode() const;
        const std::string &getHttpVersion() const;
        // const Headers &getHeaders() const;
        const types::Option<std::string> &getBody() const;

        std::string toString() const;

    private:
        HttpStatusCode status_;
        std::string httpVersion_;
        // Headers headers_;
        types::Option<std::string> body_;
    };
} //namespace http
