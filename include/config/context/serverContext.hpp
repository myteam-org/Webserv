#pragma once

class ConfigTokenizer;
class LocationContext;

#include <sys/types.h>

#include <cstddef>
#include <iostream>
#include <map>
#include <vector>

#include "documentRootConfig.hpp"
#include "locationContext.hpp"
#include "token.hpp"
#include "tokenizer.hpp"
#include "status.hpp"

class ServerContext {
   public:
    static const int PAGE_NUMBER = 404;
    static const int MAX_BODY_SIZE = 1048576;
    explicit ServerContext(const std::string& text);
    ~ServerContext();

    std::string newToken(std::string& token);
    void setListen(u_int16_t port);
    void setHost(const std::string& host);
    void setserverName(const std::string& serverName);
    void addMap(http::HttpStatusCode number, const std::string& fileName);
    void setClientMaxBodySize(size_t size);
    void addLocation(const LocationContext& location);

    const std::string& getValue() const;
    u_int16_t getListen() const;
    const std::string& getHost() const;
    const std::string& getServerName() const;
    const std::vector<std::map<http::HttpStatusCode, std::string> >& getErrorPage() const;
    size_t getClientMaxBodySize() const;
    std::vector<LocationContext>& getLocation();
    const std::vector<LocationContext>& getLocation() const;

   private:
    std::string value_;
    u_int16_t listen_;
    std::string host_;
    std::string serverName_;
    std::vector<std::map<http::HttpStatusCode, std::string> > errorPage_;
    size_t clientMaxBodySize_;
    std::vector<LocationContext> locations_;
};
