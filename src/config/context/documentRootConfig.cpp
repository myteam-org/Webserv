#include "documentRootConfig.hpp"

DocumentRootConfig::DocumentRootConfig()
    : autoIndex_(OFF), cgiExtensions_(OFF), index_("wildCard") {}

DocumentRootConfig::~DocumentRootConfig() {}

void DocumentRootConfig::setRoot(const std::string& root) {
    this->root_ = root;
}

void DocumentRootConfig::setIndex(const std::string& index) {
    this->index_ = index;
}

void DocumentRootConfig::setAutoIndex(OnOff autoIndex) {
    this->autoIndex_ = autoIndex;
}

void DocumentRootConfig::setCgiExtensions(OnOff cgiExtensions) {
    this->cgiExtensions_ = cgiExtensions;
}

const std::string& DocumentRootConfig::getRoot() const { return (this->root_); }

const std::string& DocumentRootConfig::getIndex() const {
    return (this->index_);
}

OnOff DocumentRootConfig::getAutoIndex() const { return (this->autoIndex_); }

OnOff DocumentRootConfig::getCgiExtensions() const {
    return (this->cgiExtensions_);
}
