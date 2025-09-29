#pragma once

class ConfigTokenizer;
class LocationContext;

#include <sys/types.h>
#include <cstddef>
#include <iostream>
#include <map>
#include <vector>

#include "config/context/documentRootConfig.hpp"
#include "config/context/locationContext.hpp"
#include "config/token.hpp"
#include "config/tokenizer.hpp"
#include "http/status.hpp"

typedef std::vector<LocationContext> LocationContextList;

class ServerContext {
   public:
    static const int PAGE_NUMBER = 404;
    static const int MAX_BODY_SIZE = 1048576;
    explicit ServerContext(const std::string& text);
    ~ServerContext();

    std::string newToken(std::string& token);
    void setListen(u_int16_t port);
    void setHost(const std::string& host);
    void addServerName(const std::string& serverName);
    void addMap(http::HttpStatusCode number, const std::string& fileName);
    void setClientMaxBodySize(size_t size);
    void addLocation(const LocationContext& location);

    const std::string& getValue() const;
    u_int16_t getListen() const;
    const std::string& getHost() const;
    const std::vector<std::string>& getServerNames() const;
    const std::map<http::HttpStatusCode, std::string>& getErrorPage() const;
    size_t getClientMaxBodySize() const;

    LocationContextList& getLocation();
    const LocationContextList& getLocation() const;

   private:
    std::string value_;
    u_int16_t listen_;
    std::string host_;
    std::vector<std::string> serverNames_;
    std::map<http::HttpStatusCode, std::string> errorPage_;
    size_t clientMaxBodySize_;
    LocationContextList locations_;
};
