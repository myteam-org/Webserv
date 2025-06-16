#pragma once

class ConfigTokenizer;
class LocationContext;

#include <sys/types.h>

#include <cstddef>
#include <iostream>
#include <map>
#include <vector>

#include "tokenizer.hpp"
#include "location.hpp"
#include "document_root.hpp"
#include "token.hpp"

class ServerContext {
       public:
        explicit ServerContext(const std::string& text);
        ~ServerContext();

        std::string newToken(std::string& token);
        void setListen(u_int16_t port);
        void setHost(const std::string& host);
        void addMap(int number, const std::string& fileName);
        void setClientMaxBodySize(size_t size);

        const std::string& getValue() const;
        u_int16_t getListen() const;
        const std::string& getHost() const;
        const std::vector<std::map<int, std::string> >& getErrorPage() const;
        size_t getClientMaxBodySize() const;
        std::vector<LocationContext>& getLocation();
        const std::vector<LocationContext>& getLocation() const;

       private:
        std::string value_;
        u_int16_t listen_;
        std::string host_;
        std::vector<std::map<int, std::string> > errorPage_;
        size_t clientMaxBodySize_;
        std::vector<LocationContext> locations_;
};
