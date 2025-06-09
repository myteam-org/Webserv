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
                std::cout << " |- listen: " << (int)server[i].getListen()
                          << std::endl;
                std::cout << " |- host: " << server[i].getHost() << std::endl;
                if (server[i].getClientMaxBodySize())
                        std::cout << " |- client_max_body_size: "
                                  << server[i].getClientMaxBodySize()
                                  << std::endl;
                const std::vector<std::map<int, std::string> >& errorPages =
                    server[i].getErrorPage();
                for (size_t j = 0; j < errorPages.size(); ++j) {
                        const std::map<int, std::string>& pageMap =
                            errorPages[j];
                        for (std::map<int, std::string>::const_iterator it =
                                 pageMap.begin();
                             it != pageMap.end(); ++it)
                                std::cout << " |- error_page: " << it->first
                                          << " -> " << it->second << std::endl;
                }
        }
}

// void Config::printParser() {
//         printParserRecursion(Config::getParser().getRoot(), 0);
// }

// void Config::printParserRecursion(ConfigNode* node, int depth) {
//         if (!node) return;

//         for (int i = 0; i < depth * 2; ++i) std::cout << " ";
//         if (depth > 0) std::cout << "|- ";
//         std::cout << node->getKey();
//         for (std::vector<std::string>::iterator iter =
//                  node->getValues().begin();
//              iter != node->getValues().end(); ++iter)
//                 std::cout << " " << *iter;
//         std::cout << std::endl;
//         for (std::vector<ConfigNode*>::iterator it =
//                  node->getChildren().begin();
//              it != node->getChildren().end(); ++it)
//                 printParserRecursion(*it, depth + 1);
// }
