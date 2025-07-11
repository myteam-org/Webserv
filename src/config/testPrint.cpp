#include <cstddef>

#include "config/config.hpp"
#include "data.hpp"
#include "documentRootConfig.hpp"
#include "locationContext.hpp"
#include "serverContext.hpp"

void Config::printServer(const std::vector<ServerContext>& server) {
    for (size_t i = 0; i < server.size(); ++i) {
        std::cout << server[i].getValue() << std::endl;
        std::cout << " |- listen: " << static_cast<int>(server[i].getListen())
                  << std::endl;
        std::cout << " |- host: " << server[i].getHost() << std::endl;
        std::cout << " |- server_name: " << server[i].getServerName()
                  << std::endl;
        if (server[i].getClientMaxBodySize()) {
            std::cout << " |- client_max_body_size: "
                      << server[i].getClientMaxBodySize() << std::endl;
        }
        const std::vector<std::map<int, std::string> >& errorPages =
            server[i].getErrorPage();
        for (size_t j = 0; j < errorPages.size(); ++j) {
            const std::map<int, std::string>& pageMap = errorPages[j];
            for (std::map<int, std::string>::const_iterator it =
                     pageMap.begin();
                 it != pageMap.end(); ++it) {
                std::cout << " |- error_page: " << it->first << " -> "
                          << it->second << std::endl;
            }
        }
        printLocation(server[i]);
        std::cout << std::endl;
    }
}

void Config::printLocation(const ServerContext& server) {
    const std::vector<LocationContext>& location = server.getLocation();
    for (size_t k = 0; k < location.size(); ++k) {
        const DocumentRootConfig& documentRootConfig =
            location[k].getDocumentRootConfig();
        std::cout << " |- location: " << location[k].getPath() << std::endl;
        if (!documentRootConfig.getRoot().empty()) {
            std::cout << "     |- root: " << documentRootConfig.getRoot()
                      << std::endl;
        }
        const OnOff* method = location[k].getAllowedMethod();
        std::cout << "     |- method: GET = " << method[GET]
                  << " POST = " << method[POST]
                  << " DELETE = " << method[DELETE] << std::endl;
        std::cout << "     |- index: " << documentRootConfig.getIndex()
                  << std::endl;
        std::cout << "     |- auto_index: " << documentRootConfig.getAutoIndex()
                  << std::endl;
        std::cout << "     |- is_cgi: " << documentRootConfig.getCgiExtensions()
                  << std::endl;
        if (!location[k].getRedirect().empty()) {
            std::cout << "     |- redirect: " << location[k].getRedirect()
                      << std::endl;
        }
    }
}
