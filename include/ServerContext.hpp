#pragma once

class ConfigTokenizer;

#include "ConfigTokenizer.hpp"
#include "Token.hpp"

#include <iostream>
#include <sys/types.h>
#include <cstddef>
#include <map>
#include <vector>

class ServerContext {
       public:
        ServerContext(const std::string& text);
        ~ServerContext();
        void setListen(u_int16_t port);
        void setHost(const std::string& host);
        void addMap(int number, const std::string& fileName);
        void setClientMaxBodySize(size_t size);
        // void addLocation();
        const std::string& getValue() const;
        u_int16_t getListen() const;
        const std::string& getHost() const;
        const std::vector<std::map<int, std::string> >& getErrorPage() const;
        size_t getClientMaxBodySize() const;
        // const LocationContext& getLocation(const std::string& path) const;

       private:
        std::string value_;
        u_int16_t listen_;
        std::string host_;
        std::vector<std::map<int, std::string> > errorPage_;
        size_t clientMaxBodySize_;
        // std::vector<LocationContext> locations_;
};
