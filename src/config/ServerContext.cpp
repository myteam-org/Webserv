#include "ServerContext.hpp"

ServerContext::ServerContext(const std::string& text)
    : value_(text), listen_(0), clientMaxBodySize_(0) {}

ServerContext::~ServerContext() {}

void ServerContext::setListen(u_int16_t port) { this->listen_ = port; }

void ServerContext::setHost(const std::string& host) { this->host_ = host; }

void ServerContext::addMap(int number, const std::string& fileName) {
        std::map<int, std::string> errPage;
        errPage.insert(std::make_pair(number, fileName));
        this->errorPage_.push_back(errPage);
}

void ServerContext::setClientMaxBodySize(size_t size) {
        this->clientMaxBodySize_ = size;
}

// void ServerContext::addLocation() {}

const std::string& ServerContext::getValue() const { return (this->value_); }

u_int16_t ServerContext::getListen() const { return (this->listen_); }

const std::string& ServerContext::getHost() const { return (this->host_); }

const std::vector<std::map<int, std::string> >& ServerContext::getErrorPage()
    const {
        return (this->errorPage_);
}

size_t ServerContext::getClientMaxBodySize() const {
        return (this->clientMaxBodySize_);
}
