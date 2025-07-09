#include "http/request/request.hpp"

namespace http {
    Request::Request(
        HttpMethod method,
        const std::string &requestTarget,// NOLINT(bugprone-easily-swappable-parameters)
        const std::string &httpVersion,
        const std::string &body
    ) : method_(method), requestTarget_(requestTarget), httpVersion_(httpVersion), body_(body) {}

    bool Request::operator==(const Request &other) const {
        return method_ == other.method_ &&
               requestTarget_ == other.requestTarget_ &&
               httpVersion_ == other.httpVersion_ &&
               body_ == other.body_;
    }

    HttpMethod Request::getMethod() const {
        return method_;
    }

    const std::string &Request::getRequestTarget() const {
        return requestTarget_;
    }

    const std::string &Request::getHttpVersion() const {
        return httpVersion_;
    }

    types::Option<std::string> Request::getHeader(const std::string &key) {
        (void)key; // Unused parameter
        return types::none<std::string>();
    }

    const std::string &Request::getBody() const {
        return body_;
    }
} // namespace http
