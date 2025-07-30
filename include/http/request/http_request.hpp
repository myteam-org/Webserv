#pragma once
#include <string>
#include <vector>

#include "context/locationContext.hpp"
#include "header.hpp"  // RawHeaders を含む
#include "raw_headers.hpp"

namespace http {

class HttpRequest {
   public:
    explicit HttpRequest();
    ~HttpRequest();
    void setMethod(const std::string& method);
    void setUri(const std::string& uri);
    void setPath(const std::string& path);
    void setQueryString(const std::string& query);
    void setVersion(const std::string& version);
    void setHeaders(const RawHeaders& headers);
    void setBody(const std::vector<char>& body);
    void setServer(const ServerContext& server);
    void setLocation(const LocationContext& location);
    void setDocumentRootConfig(const DocumentRootConfig& getDocumentRoot);
    const std::string& getMethod() const;
    const std::string& getRequestTarget() const;
    const std::string& getPath() const;
    const std::string& getQueryString() const;
    const std::string& getVersion() const;
    const RawHeaders& getHeaders() const;
    const std::vector<char>& getBody() const;
    const std::string& getHost() const;
    const ServerContext* getServer() const;
    const LocationContext* getLocation() const;
    const DocumentRootConfig* getDocumentRoot() const;

   private:
    std::string method_;
    std::string requestTarget_;
    std::string path_;
    std::string query_;
    std::string httpVersion_;
    RawHeaders headers_;
    std::vector<char> body_;
    std::string host_;
    const ServerContext* server_;
    const LocationContext* location_;
    const DocumentRootConfig* documentRoot_;
};

}  // namespace http