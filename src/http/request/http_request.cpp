#include "http_request.hpp"

namespace http {
    HttpRequest::HttpRequest() {}

    HttpRequest::~HttpRequest() {}
    
    void HttpRequest::setMethod(const std::string& method) {
        method_ = method;
    }

    void HttpRequest::setUri(const std::string& uri) {
        requestTarget_ = uri;
    }

    void HttpRequest::setPath(const std::string& path) {
        path_ = path;
    }
    
    void HttpRequest::setQueryString(const std::string& query) {
        query_ = query;
    }
    
    void HttpRequest::setVersion(const std::string& version) {
        httpVersion_ = version;
    }

    void HttpRequest::setHeaders(const RawHeaders& headers) {
        headers_ = headers;
    }

    void HttpRequest::setBody(const std::vector<char>& body) {
        body_ = body;
    }

    void HttpRequest::setServer(const ServerContext& server) {
        server_ = &server;
    }

    void HttpRequest::setLocation(const LocationContext& location) {
        location_ = &location;
    }

    void HttpRequest::setDocumentRootConfig(const DocumentRootConfig& documentRoot) {
        documentRoot_ = &documentRoot;
    }
    
    const std::string& HttpRequest::getMethod() const {
        return method_;
    }

    const std::string& HttpRequest::getRequestTarget() const {
        return requestTarget_;
    }

    const std::string& HttpRequest::getPath() const {
        return path_;
    }

    const std::string& HttpRequest::getQueryString() const {
        return query_;
    }

    const std::string& HttpRequest::getVersion() const {
        return httpVersion_;
    }

    const RawHeaders& HttpRequest::getHeaders() const {
        return headers_;
    }

    const std::vector<char>& HttpRequest::getBody() const {
        return body_;
    }

    const std::string& HttpRequest::getHost() const {
        return server_->getHost();
    }

    const ServerContext* HttpRequest::getServer() const {
        return server_;
    }

    const LocationContext* HttpRequest::getLocation() const {
        return location_;
    }

    const DocumentRootConfig* HttpRequest::getDocumentRoot() const {
        return documentRoot_;
    }

} // namespace http