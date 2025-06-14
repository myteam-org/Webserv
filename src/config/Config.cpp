#include "Config.hpp"

#include <cstddef>

#include "LocationContext.hpp"
#include "ServerContext.hpp"
#include "DocumentRootConfig.hpp"
#include "data.hpp"

Config::Config(const std::string& filename)
    : tokenizer_(const_cast<std::string&>(filename)), parser_(tokenizer_) {}

Config::~Config() {}

const ConfigParser& Config::getParser() const { return (this->parser_); }

void Config::printParser() const {
        printServer(Config::getParser().getServer());
}

void Config::printServer(const std::vector<ServerContext>& server) {
        for (size_t i = 0; i < server.size(); ++i) {
                std::cout << server[i].getValue() << std::endl;
                std::cout << " |- listen: "
                          << static_cast<int>(server[i].getListen())
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
                printLocation(server[i]);
                std::cout << std::endl;
        }
}

void Config::printLocation(const ServerContext& server) {
        const std::vector<LocationContext>& location = server.getLocation();
        for (size_t k = 0; k < location.size(); ++k) {
                const DocumentRootConfig& docmentRoot = location[k].getDocRootConfig();
                std::cout << " |- location: " << location[k].getPath()
                          << std::endl;
                if (!docmentRoot.getRoot().empty()) {
                        std::cout << "     |- root: " << docmentRoot.getRoot()
                                  << std::endl;
                }
                std::cout << "     |- method: ";
                const std::vector<AllowedMethod> method =
                    location[k].getMethod();
                for (size_t l = 0; l < method.size(); ++l) {
                        std::cout << method[l] << " ";
                }
                std::cout << std::endl;
                std::cout << "     |- index: " << docmentRoot.getIndex()
                          << std::endl;
                std::cout << "     |- auto_index: "
                          << docmentRoot.getAutoIndex() << std::endl;
                std::cout << "     |- is_cgi: " << docmentRoot.getCgiExtensions()
                          << std::endl;
                if (!location[k].getRedirect().empty()) {
                        std::cout
                            << "     |- redirect: " << location[k].getRedirect()
                            << std::endl;
                }
        }
}

bool Config::checkArgc(int argc) {
        if (argc > 2) {
                std::cerr << "Argument error" << std::endl;
                return (false);
        }
        return (true);
}

std::string Config::setFile(int argc, char** argv) {
        std::string confFile;
        if (argc == 1) {
                confFile = FILE_NAME;
        } else {
                confFile = argv[1];
        }
        return (confFile);
}

void Config::checkFile(std::string& filename) {
        struct stat status;

        if (stat(filename.c_str(), &status) != 0) {
                std::cerr << "Failed to stat file: " << filename << std::endl;
                std::exit(1);
        }
        if (status.st_mode & S_IFDIR) {
                std::cerr << filename << ": is a directory" << std::endl;
                std::exit(1);
        }
}
