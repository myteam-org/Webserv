#pragma once

#include <iostream>

#include "ConfigParser.hpp"
#include "Token.hpp"
#include "data.hpp"

class LocationContext {
       public:
        explicit LocationContext(const std::string& text);
        ~LocationContext();

        void setPath(const std::string& path);
        void setRoot(const std::string& root);
        void addMethod(AllowedMethod method);
        void setIndex(const std::string& indexPage);
        void setAutoIndex(OnOff autoIndex);
        void setIsCgi(OnOff isCgi);
        void setRedirect(const std::string& redirect);
        const std::string& getPath() const;
        const std::string& getRoot() const;
        std::vector<AllowedMethod> getMethod();
        const std::vector<AllowedMethod>& getMethod() const;
        const std::string& getIndex() const;
        OnOff getAutoIndex() const;
        OnOff getIsCgi() const;
        const std::string& getRedirect() const;
        // const DocumentRootConfig& getDocRootConfig() const;

       private:
        const std::string value_;
        std::string path_;
        std::string root_;
        std::vector<AllowedMethod> allowedMethod_;
        std::string index_;
        OnOff autoIndex_;
        OnOff isCgi_;
        std::string redirect_;
        // DocumentRootConfig docRootConfig_;
};
