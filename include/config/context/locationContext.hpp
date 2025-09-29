#pragma once

#include <string>
#include <vector>
#include "config/data.hpp"
#include "config/context/documentRootConfig.hpp"
#include "http/method.hpp"

class LocationContext {
public:
    LocationContext(const std::string& text);
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
    std::vector<http::HttpMethod> getAllowedMethods() const; // ← vectorでHttpMethod型を使う

private:
    std::string value_;
    std::string path_;
    OnOff allowedMethod_[METHOD_COUNT];
    std::string redirect_;
    DocumentRootConfig documentRootConfig_;
};
