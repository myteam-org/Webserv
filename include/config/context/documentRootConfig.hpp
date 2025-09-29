#pragma once

#include <string>
#include "utils/types/result.hpp"
#include "config/data.hpp"

class DocumentRootConfig {
public:
    DocumentRootConfig();
    ~DocumentRootConfig();

    void setRoot(const std::string& root);
    void setIndex(const std::string& index);
    void setAutoIndex(OnOff autoIndex);
    void setCgiExtensions(OnOff cgiExtensions);
    void setEnableUpload(OnOff enableUpload);
    void setLocationPath(const std::string& path);

    const std::string& getRoot() const;
    const std::string& getIndex() const;
    OnOff getAutoIndex() const;
    OnOff getCgiExtensions() const;
    OnOff getEnableUpload() const;
    const std::string& getLocationPath() const;

    bool isAutoindexEnabled() const;

private:
    std::string root_;
    std::string index_;
    OnOff autoIndex_;
    OnOff cgiExtensions_;
    OnOff enableUpload_;
    std::string locationPath_;
};
