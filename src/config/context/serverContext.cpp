#include "config/context/serverContext.hpp"

ServerContext::ServerContext(const std::string& text)
    : value_(text), listen_(0), clientMaxBodySize_(MAX_BODY_SIZE), host_("127.0.0.1") {
    // std::map<http::HttpStatusCode, std::string> errPage;
    // errPage.insert(std::make_pair(http::kStatusNotFound, "404.html"));
    // this->errorPage_.push_back(errPage);
    errorPage_[http::kStatusNotFound] = "404.html";
}

ServerContext::~ServerContext() {}

void ServerContext::setListen(u_int16_t port) { this->listen_ = port; }

void ServerContext::setHost(const std::string& host) { this->host_ = host; }

void ServerContext::setserverName(const std::string& serverName) {
    this->serverName_ = serverName;
}

void ServerContext::addMap(http::HttpStatusCode number, const std::string& fileName) {
    // std::map<http::HttpStatusCode, std::string> errPage;
    // errPage.insert(std::make_pair(number, fileName));
    // this->errorPage_.push_back(errPage);
    errorPage_[number] = fileName;
}

void ServerContext::setClientMaxBodySize(size_t size) {
    this->clientMaxBodySize_ = size;
}

void ServerContext::addLocation(const LocationContext& location) {
    locations_.push_back(location);
}

const std::string& ServerContext::getValue() const { return (this->value_); }

u_int16_t ServerContext::getListen() const { return (this->listen_); }

const std::string& ServerContext::getHost() const { return (this->host_); }

const std::string& ServerContext::getServerName() const {
    return (this->serverName_);
}

const std::map<http::HttpStatusCode, std::string>& ServerContext::getErrorPage()
    const {
    return (this->errorPage_);
}

size_t ServerContext::getClientMaxBodySize() const {
    return (this->clientMaxBodySize_);
}

std::vector<LocationContext>& ServerContext::getLocation() {
    return (this->locations_);
}

const std::vector<LocationContext>& ServerContext::getLocation() const {
    return (this->locations_);
}
