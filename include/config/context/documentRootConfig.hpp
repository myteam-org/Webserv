#pragma once

#include <iostream>
#include "config/data.hpp"
#include "config/parser.hpp"
#include "config/token.hpp"

class DocumentRootConfig {
   public:
    explicit DocumentRootConfig();
    ~DocumentRootConfig();

    void setRoot(const std::string& root);
    void setIndex(const std::string& index);
    void setAutoIndex(OnOff autoIndex);
    void setCgiExtensions(OnOff cgiExtensions);
    void setEnableUpload(OnOff enableUpload);

    const std::string& getRoot() const;
    const std::string& getIndex() const;
    OnOff getAutoIndex() const;
    OnOff getCgiExtensions() const;
    OnOff getEnableUpload() const;

    bool isAutoindexEnabled() const;

   private:
    std::string root_;
    std::string index_;
    OnOff autoIndex_;
    OnOff cgiExtensions_;
    OnOff enableUpload_;
};
