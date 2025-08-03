#pragma once

// #include "serverContext.hpp"
// #include "locationContext.hpp"
#include "option.hpp"
#include "method.hpp"
#include "raw_headers.hpp"
#include <map>
#include <vector>
#include <string>

class ServerContext;
class LocationContext;
class DocumentRootConfig;

namespace http {
  class Request {
  public:
    explicit Request(
      HttpMethod method,
      const std::string &requestTarget,
      const std::string &pathOnly,
      const std::string &queryString,
      const RawHeaders &headers,
      const std::vector<char> &body,
      const ServerContext *server,
      const LocationContext *location
    );

    bool operator==(const Request &other) const;

    HttpMethod getMethod() const;
    const std::string &getRequestTarget() const;
    const std::string& getPath() const;
    const std::string& getQueryString() const;
    const std::string &getHttpVersion() const;
    types::Option<std::string> getHeader(const std::string &key);
    const std::vector<char> &getBody() const;
    const ServerContext* getServer() const;
    const LocationContext* getLocation() const;
    const DocumentRootConfig* getDocumentRoot() const;

  private:
    HttpMethod method_;
    std::string requestTarget_;
    std::string pathOnly_;
    std::string queryString_;
    std::string httpVersion_;
    RawHeaders headers_;
    std::vector<char> body_;
    const ServerContext* server_;
    const LocationContext* location_;
    const DocumentRootConfig* documentRoot_;
  };
} // namespace http
