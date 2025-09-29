#include "config/context/serverContext.hpp"

ServerContext::ServerContext(const std::string& text)
    : value_(text),
      listen_(0),
      clientMaxBodySize_(MAX_BODY_SIZE),
      host_("localhost"),
      serverNames_() // vector初期化
{
    errorPage_[http::kStatusNotFound] = "404.html";
}

ServerContext::~ServerContext() {}

void ServerContext::setListen(u_int16_t port) {
    this->listen_ = port;
}

void ServerContext::setHost(const std::string& host) {
    this->host_ = host;
}

// server_names（複数対応）追加
void ServerContext::addServerName(const std::string& serverName) {
    this->serverNames_.push_back(serverName);
}

// エラーページ設定
void ServerContext::addMap(http::HttpStatusCode number, const std::string& fileName) {
    errorPage_[number] = fileName;
}

void ServerContext::setClientMaxBodySize(size_t size) {
    this->clientMaxBodySize_ = size;
}

void ServerContext::addLocation(const LocationContext& location) {
    locations_.push_back(location);
}

// サーバーブロックの生テキスト取得
const std::string& ServerContext::getValue() const {
    return (this->value_);
}

u_int16_t ServerContext::getListen() const {
    return (this->listen_);
}

const std::string& ServerContext::getHost() const {
    return (this->host_);
}

// server_names（複数対応）取得
const std::vector<std::string>& ServerContext::getServerNames() const {
    return (this->serverNames_);
}

const std::map<http::HttpStatusCode, std::string>& ServerContext::getErrorPage() const {
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
