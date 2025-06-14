#pragma once

#include <iostream>

#include "ConfigParser.hpp"
#include "DocumentRootConfig.hpp"
#include "Token.hpp"
#include "data.hpp"

class LocationContext {
       public:
        explicit LocationContext(const std::string& text);
        ~LocationContext();

        void setPath(const std::string& path);
        // void addMethod(AllowedMethod method);
        void setMethod(AllowedMethod method);
        void setRedirect(const std::string& redirect);
        const std::string& getPath() const;
        // std::vector<AllowedMethod> getMethod();
        // const std::vector<AllowedMethod>& getMethod() const;
        OnOff* getMethod();
        const OnOff* getMethodArray() const;
        const std::string& getRedirect() const;
        DocumentRootConfig& getDocumentRootConfig();
        const DocumentRootConfig& getDocumentRootConfig() const;

       private:
        const std::string value_;
        std::string path_;
        // std::vector<AllowedMethod> allowedMethod_;
        OnOff allowedMethod_[3];
        std::string redirect_;
        DocumentRootConfig documentRootConfig_;
};
