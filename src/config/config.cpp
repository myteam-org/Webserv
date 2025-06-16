#include "config/config.hpp"

#include <cstddef>

#include "location.hpp"
#include "server.hpp"
#include "document_root.hpp"
#include "data.hpp"

struct ConfigServerValueErrorEraser{
       private:
        const Config* config;

       public:
        explicit ConfigServerValueErrorEraser(const Config* cfg) : config(cfg) {}

        bool operator()(const ServerContext& server) const {
                return (server.getHost().empty()
                        || server.getListen() == 0
                        || Config::checkAndEraseLocationNode(server)
			|| server.getLocation().empty());
        }
};

struct ConfigServerDuplicateErrorEraser {
        private:
        std::vector<int>* seenListenValue;

        public:
        explicit ConfigServerDuplicateErrorEraser(std::vector<int>* seen) : seenListenValue(seen) {}

        bool operator()(const ServerContext& server) {
                const int listen = server.getListen();

                for (std::vector<int>::iterator it = seenListenValue->begin(); it != seenListenValue->end(); ++it) {
                        std::cerr << "[ server removed: listen port duplicate error ]" << std::endl;
                        if (*it == listen) {
                                return (true);
                        }
                }
                seenListenValue->push_back(listen);
                return (false);
        }
};

Config::Config(const std::string& filename)
    : tokenizer_(const_cast<std::string&>(filename)), parser_(tokenizer_) {
        checkAndEraseServerNode();
        removeDuplicateListenServers(this->parser_.getServer());
    }

Config::~Config() {}

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

void Config::checkAndEraseServerNode() {
        std::vector<ServerContext>& servers = this->parser_.getServer();

        servers.erase(
                std::remove_if(servers.begin(), servers.end(), 
                        ConfigServerValueErrorEraser(this)), servers.end());
}

bool Config::checkAndEraseLocationNode(const ServerContext& server) {
        for (std::vector<LocationContext>::const_iterator it = server.getLocation().begin();
                it != server.getLocation().end(); ++it) {
                        return (it->getPath().empty() 
                                || it->getDocumentRootConfig().getRoot().empty());
                }
        return (false);
}

void Config::removeDuplicateListenServers(std::vector<ServerContext>& servers) {
        std::vector<int> seenListenValue;

        servers.erase(
                std::remove_if(servers.begin(), servers.end(), ConfigServerDuplicateErrorEraser(&seenListenValue)),
                        servers.end());
}

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
                const DocumentRootConfig& documentRootConfig = location[k].getDocumentRootConfig();
                std::cout << " |- location: " << location[k].getPath()
                          << std::endl;
                if (!documentRootConfig.getRoot().empty()) {
                        std::cout << "     |- root: " << documentRootConfig.getRoot()
                                  << std::endl;
                }
                const OnOff* method = location[k].getAllowedMethod();
                std::cout << "     |- method: GET = " << method[GET] << " POST = " << method[POST] 
                          << " DELETE = " << method[DELETE] << std::endl;
                std::cout << "     |- index: " << documentRootConfig.getIndex()
                          << std::endl;
                std::cout << "     |- auto_index: "
                          << documentRootConfig.getAutoIndex() << std::endl;
                std::cout << "     |- is_cgi: " << documentRootConfig.getCgiExtensions()
                          << std::endl;
                if (!location[k].getRedirect().empty()) {
                        std::cout
                            << "     |- redirect: " << location[k].getRedirect()
                            << std::endl;
                }
        }
}
