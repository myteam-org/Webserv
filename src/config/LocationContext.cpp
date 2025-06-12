#include "LocationContext.hpp"

#include <string>

LocationContext::LocationContext(const std::string& text)
    : value_(text), autoIndex_(OFF), isCgi_(OFF) {}

LocationContext::~LocationContext() {}

void LocationContext::setPath(const std::string& path) { this->path_ = path; }

void LocationContext::setRoot(const std::string& root) { this->root_ = root; }

void LocationContext::addMethod(AllowedMethod method) {
        this->allowedMethod_.push_back(method);
}

void LocationContext::setIndex(const std::string& indexPage) {
        this->index_ = indexPage;
}

void LocationContext::setAutoIndex(OnOff autoIndex) {
        this->autoIndex_ = autoIndex;
}

void LocationContext::setIsCgi(OnOff isCgi) { this->isCgi_ = isCgi; }

void LocationContext::setRedirect(const std::string& redirect) {
        this->redirect_ = redirect;
}

const std::string& LocationContext::getPath() const { return (this->path_); }

const std::string& LocationContext::getRoot() const { return (this->root_); }

std::vector<AllowedMethod> LocationContext::getMethod() {
        return (this->allowedMethod_);
}

const std::vector<AllowedMethod>& LocationContext::getMethod() const {
        return (this->allowedMethod_);
}

const std::string& LocationContext::getIndex() const { return (this->index_); }

OnOff LocationContext::getAutoIndex() const { return (this->autoIndex_); }

OnOff LocationContext::getIsCgi() const { return (this->isCgi_); }

const std::string& LocationContext::getRedirect() const {
        return (this->redirect_);
}
