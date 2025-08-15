#include "http/response/response.hpp"

#include "http/response/response_headers.hpp"
#include <iostream>
#include <sstream>

namespace http {
const std::string Response::defaultHttpVersion = "HTTP/1.1";

Response::Response(HttpStatusCode status,
                   const types::Option<std::string> &body,
                   const std::string &httpVersion)
    : status_(status), httpVersion_(httpVersion), body_(body) {}

Response::Response(HttpStatusCode status, const ResponseHeaderFields &headers,
                   const types::Option<std::string> &body,
                   const std::string &httpVersion)
    : status_(status),
      headers_(headers),
      body_(body),
      httpVersion_(httpVersion) {}

bool Response::operator==(const Response &other) const {
    return status_ == other.status_ && httpVersion_ == other.httpVersion_ &&
           body_ == other.body_;
}

HttpStatusCode Response::getStatusCode() const { return status_; }

const ResponseHeaderFields &Response::getHeaders() const { return headers_; }

const std::string &Response::getHttpVersion() const { return httpVersion_; }

const types::Option<std::string> &Response::getBody() const { return body_; }

std::string Response::toString() {
    std::ostringstream out;

    const std::string reason = getHttpStatusText(status_);
    out << httpVersion_ << ' ' << static_cast<int>(status_);
    if (!reason.empty()) {
        out << ' ' << reason;
    }
    out << "\r\n";

    ResponseHeaderFields::const_iterator it = headers_.begin();
    for (; it != headers_.end(); ++it) {
        out << it->first << ": " << it->second << "\r\n";
    }
    out << "\r\n";
    if (!body_.isNone()) {
        out << body_.unwrap();
    }
    return out.str();
}
}  // namespace http
