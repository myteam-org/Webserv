#include "http/response/response.hpp"

namespace http {
    Response::Response(
        HttpStatusCode status,
        types::Option<std::string> body,
        const std::string &httpVersion
    ) : status_(status), httpVersion_(httpVersion), body_(body) {}

    bool Response::operator==(const Response &other) const {
        return status_ == other.status_ &&
               httpVersion_ == other.httpVersion_ &&
               body_ == other.body_;
    }

    HttpStatusCode Response::getStatusCode() const {
        return status_;
    }

    const std::string &Response::getHttpVersion() const {
        return httpVersion_;
    }

    const types::Option<std::string> &Response::getBody() const {
        return body_;
    }

    std::string Response::toString() const {
        // Implementation missing
        return "";
    }
}
