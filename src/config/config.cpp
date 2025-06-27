#include "config/config.hpp"

#include <cstddef>

#include "data.hpp"
#include "document_root.hpp"
#include "location.hpp"
#include "server.hpp"

struct ConfigServerValueErrorEraser {
   private:
    const Config* config;

   public:
    explicit ConfigServerValueErrorEraser(const Config* cfg) : config(cfg) {}

    bool operator()(const ServerContext& server) const {
        return (server.getHost().empty() || server.getListen() == 0 ||
                Config::checkAndEraseLocationNode(server) ||
                server.getLocation().empty());
    }
};

struct ConfigServerDuplicateErrorEraser {
   private:
    std::vector<int>* seenListenValue;

   public:
    explicit ConfigServerDuplicateErrorEraser(std::vector<int>* seen)
        : seenListenValue(seen) {}

    bool operator()(const ServerContext& server) {
        const int listen = server.getListen();

        for (std::vector<int>::iterator it = seenListenValue->begin();
             it != seenListenValue->end(); ++it) {
            if (*it == listen) {
                std::cerr << "[ server removed: listen port "
                             "duplicate error ]"
                          << std::endl;
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

void Config::checkFile(const std::string& filename) {
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

    servers.erase(std::remove_if(servers.begin(), servers.end(),
                                 ConfigServerValueErrorEraser(this)),
                  servers.end());
    std::cerr << "[ server removed: server block member error ]"
              << std::endl;
}

bool Config::checkAndEraseLocationNode(const ServerContext& server) {
    for (std::vector<LocationContext>::const_iterator it =
             server.getLocation().begin();
         it != server.getLocation().end(); ++it) {
        return (it->getPath().empty() ||
                it->getDocumentRootConfig().getRoot().empty());
    }
    return (false);
}

void Config::removeDuplicateListenServers(std::vector<ServerContext>& servers) {
    std::vector<int> seenListenValue;

    servers.erase(
        std::remove_if(servers.begin(), servers.end(),
                       ConfigServerDuplicateErrorEraser(&seenListenValue)),
        servers.end());
}

const ConfigParser& Config::getParser() const { return (this->parser_); }

void Config::printParser() const {
    printServer(Config::getParser().getServer());
}
