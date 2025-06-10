#include "Config.hpp"

#include "ServerContext.hpp"

Config::Config(const std::string& filename)
    : tokenizer_(const_cast<std::string&>(filename)), parser_(tokenizer_) {
        // checkTree(); TODO
}

Config::~Config() {}

ConfigTokenizer& Config::getTokenizer() { return (this->tokenizer_); }

const ConfigParser& Config::getParser() const { return (this->parser_); }

void Config::printParser() { printServer(Config::getParser().getServr()); }

void Config::printServer(const std::vector<ServerContext>& server) {
        for (size_t i = 0; i < server.size(); ++i) {
                std::cout << server[i].getValue() << std::endl;
                std::cout << " |- listen: " << static_cast<int>(server[i].getListen())
                          << std::endl;
                std::cout << " |- host: " << server[i].getHost() << std::endl;
                if (server[i].getClientMaxBodySize()) {
                    std::cout << " |- client_max_body_size: "
                              << server[i].getClientMaxBodySize()
                              << std::endl;
                }
                const std::vector<std::map<int, std::string> >& errorPages =
                    server[i].getErrorPage();
                for (size_t j = 0; j < errorPages.size(); ++j) {
                        const std::map<int, std::string>& pageMap =
                            errorPages[j];
                        for (std::map<int, std::string>::const_iterator it =
                                 pageMap.begin();
                             it != pageMap.end(); ++it) {
                                 std::cout << " |- error_page: " << it->first
                                           << " -> " << it->second << std::endl;
                             }
                }
        }
}

