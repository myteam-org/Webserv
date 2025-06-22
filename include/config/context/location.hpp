#pragma once

#include <iostream>

#include "data.hpp"
#include "document_root.hpp"
#include "parser.hpp"
#include "token.hpp"

class LocationContext {
       public:
        explicit LocationContext(const std::string& text);
        ~LocationContext();

        void setPath(const std::string& path);
        void setMethod(AllowedMethod method);
        void setRedirect(const std::string& redirect);
        const std::string& getPath() const;
        OnOff* getMutableAllowedMethod();
        const OnOff* getAllowedMethod() const;
        const std::string& getRedirect() const;
        DocumentRootConfig& getDocumentRootConfig();
        const DocumentRootConfig& getDocumentRootConfig() const;

       private:
        static const int METHOD_COUNT = 3;
        std::string value_;
        std::string path_;
        OnOff allowedMethod_[METHOD_COUNT];
        std::string redirect_;
        DocumentRootConfig documentRootConfig_;
};
