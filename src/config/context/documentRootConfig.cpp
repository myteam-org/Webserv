#include "documentRootConfig.hpp"

DocumentRootConfig::DocumentRootConfig()
    : index_("index.html"), autoIndex_(OFF), cgiExtensions_(OFF), enableUpload_(OFF) {
}

DocumentRootConfig::~DocumentRootConfig() {
}

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

bool DocumentRootConfig::isAutoindexEnabled() const {
    return autoIndex_ == ON;
}
