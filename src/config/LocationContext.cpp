#include "LocationContext.hpp"

#include <string>
#include "data.hpp"

LocationContext::LocationContext(const std::string& text) : value_(text) {
        this->allowedMethod_[GET] = OFF;
        this->allowedMethod_[POST] = OFF;
        this->allowedMethod_[DELETE] = OFF;
}

LocationContext::~LocationContext() {}

void LocationContext::setPath(const std::string& path) { this->path_ = path; }

void LocationContext::setMethod(AllowedMethod method) {
        this->allowedMethod_[method] = ON;
}

void LocationContext::setRedirect(const std::string& redirect) {
        this->redirect_ = redirect;
}

const std::string& LocationContext::getPath() const { return (this->path_); }

OnOff* LocationContext::getMutableAllowedMethod() { return (this->allowedMethod_); }

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
