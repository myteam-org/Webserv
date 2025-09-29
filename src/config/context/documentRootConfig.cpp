#include "config/context/documentRootConfig.hpp"

DocumentRootConfig::DocumentRootConfig()
    : root_(""), index_("index.html"), autoIndex_(OFF), cgiExtensions_(OFF), enableUpload_(OFF), locationPath_("") {
}

DocumentRootConfig::~DocumentRootConfig() {}

void DocumentRootConfig::setRoot(const std::string& root) {
    root_ = root;
}

void DocumentRootConfig::setIndex(const std::string& index) {
    index_ = index;
}

void DocumentRootConfig::setAutoIndex(OnOff autoIndex) {
    autoIndex_ = autoIndex;
}

void DocumentRootConfig::setCgiExtensions(OnOff cgiExtensions) {
    cgiExtensions_ = cgiExtensions;
}

void DocumentRootConfig::setEnableUpload(OnOff enableUpload) {
    enableUpload_ = enableUpload;
}

void DocumentRootConfig::setLocationPath(const std::string& path) {
    if (path.empty()) {
        locationPath_ = "/";
    } else {
        locationPath_ = path;
    }
}

const std::string& DocumentRootConfig::getRoot() const {
    return root_;
}

const std::string& DocumentRootConfig::getIndex() const {
    return index_;
}

OnOff DocumentRootConfig::getAutoIndex() const {
    return autoIndex_;
}

OnOff DocumentRootConfig::getCgiExtensions() const {
    return cgiExtensions_;
}

OnOff DocumentRootConfig::getEnableUpload() const {
    return enableUpload_;
}

const std::string& DocumentRootConfig::getLocationPath() const {
    return locationPath_;
}

bool DocumentRootConfig::isAutoindexEnabled() const {
    return autoIndex_ == ON;
}
