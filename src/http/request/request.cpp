#include "http/request/request.hpp"
#include "config/context/locationContext.hpp"

namespace http {
Request::Request(HttpMethod method, const std::string &requestTarget,
                 const std::string &pathOnly, const std::string &queryString,
                 const std::string &httpVersion, const RawHeaders &headers,
                 const std::vector<char> &body, const ServerContext *server,
                 const LocationContext *location)
    : method_(method),
      requestTarget_(requestTarget),
      pathOnly_(pathOnly),
      queryString_(queryString),
      httpVersion_(httpVersion),
      headers_(headers),
      body_(body),
      server_(server),
      location_(location) {
    documentRoot_ = &location_->getDocumentRootConfig();
}

bool Request::operator==(const Request &other) const {
    return method_ == other.method_ && requestTarget_ == other.requestTarget_ &&
           httpVersion_ == other.httpVersion_ && body_ == other.body_;
}

HttpMethod Request::getMethod() const { return method_; }

const std::string &Request::getRequestTarget() const { return requestTarget_; }

const std::string &Request::getPath() const { return pathOnly_; }

const std::string &Request::getQueryString() const { return queryString_; }

const std::string &Request::getHttpVersion() const { return httpVersion_; }

const types::Option<std::string> Request::getHeader(const std::string &key) {
    for (RawHeaders::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
        if (it != headers_.end()) {
            return types::some(it->second);
        }
        return types::none<std::string>();
    }
}

const std::vector<char>& Request::getBody() const { return body_; }

const ServerContext* Request::getServer() const { return server_; }

const LocationContext* Request::getLocation() const { return location_; }

const DocumentRootConfig* Request::getDocumentRoot() const { return documentRoot_; }
}  // namespace http
