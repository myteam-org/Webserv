#pragma once

#include <iostream>

#include "../data.hpp"
#include "documentRootConfig.hpp"
#include "../parser.hpp"
#include "../token.hpp"
#include "http/method.hpp"

typedef std::vector<LocationContext> LocationContextList;

class LocationContext {
   public:
    explicit LocationContext(const std::string& text);
    ~LocationContext();

    void setPath(const std::string& path);
    void setMethod(AllowedMethod method);
    void setRedirect(const std::string& redirect);
    const std::string& getPath() const;
    OnOff* getMutableAllowedMethod();
    const OnOff* getAllowedMethod() const;
    const std::string& getRedirect() const;
    DocumentRootConfig& getDocumentRootConfig();
    const DocumentRootConfig& getDocumentRootConfig() const;
    std::vector<http::HttpMethod> getAllowedMethods() const;

   private:
    static const int METHOD_COUNT = 3;
    std::string value_;
    std::string path_;
    OnOff allowedMethod_[METHOD_COUNT];
    std::string redirect_;
    DocumentRootConfig documentRootConfig_;
};
