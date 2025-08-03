#include "http/request/request.hpp"

#include "config/context/locationContext.hpp"
#include "utils/string.hpp"

namespace http {
Request::Request(HttpMethod method, const std::string &requestTarget,
                 const RawHeaders &headers, const std::vector<char> &body,
                 const ServerContext *server, const LocationContext *location)
    : method_(method),
      requestTarget_(requestTarget),
      headers_(headers),
      body_(body),
      server_(server),
      location_(location) {
    httpVersion_ = "HTTP/1.1";
    documentRoot_ = &location_->getDocumentRootConfig();
    splitTarget();
}

void Request::splitTarget() {
    const std::string::size_type question = requestTarget_.find('?');
    if (question == std::string::npos) {
        pathOnly_ = requestTarget_;
        queryString_.clear();
    } else {
        pathOnly_ = requestTarget_.substr(0, question);
        queryString_ = requestTarget_.substr(question + 1);
    }
}

bool Request::operator==(const Request &other) const {
    return method_ == other.method_ && requestTarget_ == other.requestTarget_ &&
           httpVersion_ == other.httpVersion_ && headers_ == other.headers_ &&
           body_ == other.body_;
}

HttpMethod Request::getMethod() const { return method_; }

const std::string &Request::getRequestTarget() const { return requestTarget_; }

const std::string &Request::getPath() const { return pathOnly_; }

const std::string &Request::getQueryString() const { return queryString_; }

const std::string &Request::getHttpVersion() const { return httpVersion_; }

types::Option<std::string> Request::getHeader(const std::string &key) {
    const std::string lowKey = utils::toLower(key);
    const RawHeaders::const_iterator it = headers_.find(lowKey);
    if (it != headers_.end()) {
        return types::some(it->second);
    }
    return types::none<std::string>();
}

const std::vector<char> &Request::getBody() const { return body_; }

const ServerContext *Request::getServer() const { return server_; }

const LocationContext *Request::getLocation() const { return location_; }

const DocumentRootConfig *Request::getDocumentRoot() const {
    return documentRoot_;
}
}  // namespace http
