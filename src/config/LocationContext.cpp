#include "LocationContext.hpp"

#include <string>

LocationContext::LocationContext(const std::string& text) : value_(text) {}

LocationContext::~LocationContext() {}

void LocationContext::setPath(const std::string& path) { this->path_ = path; }

void LocationContext::addMethod(AllowedMethod method) {
        this->allowedMethod_.push_back(method);
}

void LocationContext::setRedirect(const std::string& redirect) {
        this->redirect_ = redirect;
}

const std::string& LocationContext::getPath() const { return (this->path_); }

std::vector<AllowedMethod> LocationContext::getMethod() {
        return (this->allowedMethod_);
}

const std::vector<AllowedMethod>& LocationContext::getMethod() const {
        return (this->allowedMethod_);
}

const std::string& LocationContext::getRedirect() const {
        return (this->redirect_);
}

DocumentRootConfig& LocationContext::getDocRootConfig() {
        return (this->docRootConfig_);
}

const DocumentRootConfig& LocationContext::getDocRootConfig() const {
        return (this->docRootConfig_);
}
