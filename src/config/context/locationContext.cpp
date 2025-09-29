#include "config/context/locationContext.hpp"
#include <vector>
#include "http/method.hpp" // ← 追加: HttpMethod型宣言のヘッダ

LocationContext::LocationContext(const std::string& text)
    : value_(text), path_(""), redirect_("") {
    for (int i = 0; i < METHOD_COUNT; ++i) {
        allowedMethod_[i] = OFF;
    }
}

LocationContext::~LocationContext() {}

void LocationContext::setPath(const std::string& path) {
    if (path.empty()) {
        this->path_ = "/";
    } else {
        this->path_ = path;
    }
    this->documentRootConfig_.setLocationPath(this->path_);
}

void LocationContext::setMethod(AllowedMethod method) {
    this->allowedMethod_[method] = ON;
}

void LocationContext::setRedirect(const std::string& redirect) {
    this->redirect_ = redirect;
}

const std::string& LocationContext::getPath() const { return (this->path_); }

OnOff* LocationContext::getMutableAllowedMethod() {
    return (this->allowedMethod_);
}

const OnOff* LocationContext::getAllowedMethod() const {
    return (this->allowedMethod_);
}

const std::string& LocationContext::getRedirect() const {
    return (this->redirect_);
}

DocumentRootConfig& LocationContext::getDocumentRootConfig() {
    return (this->documentRootConfig_);
}

const DocumentRootConfig& LocationContext::getDocumentRootConfig() const {
    return (this->documentRootConfig_);
}

std::vector<http::HttpMethod> LocationContext::getAllowedMethods() const {
    std::vector<http::HttpMethod> methods;
    for (int i = 0; i < METHOD_COUNT; ++i) {
        if (allowedMethod_[i] == ON) {
            methods.push_back(static_cast<http::HttpMethod>(i));
        }
    }
    return methods;
}
