#include "virtual_server.hpp"
#include "router/router.hpp"
#include "middleware/chain.hpp"

VirtualServer::VirtualServer(const ServerContext &serverConfig, const std::string &bindAddress)
    : serverConfig_(serverConfig), bindAddress_(bindAddress) {
    this->setupRouter();
}

const ServerContext &VirtualServer::getServerConfig() const {
    return serverConfig_;
}

http::Router &VirtualServer::getRouter() {
    return router_;
}


void VirtualServer::registerHandlers(const LocationContext &location) { // NOLINT(readability-convert-member-functions-to-static)
    std::vector<http::HttpMethod> allowedMethods = location.getAllowedMethods();
    for (std::vector<http::HttpMethod>::const_iterator iter = allowedMethods.begin();
            iter != allowedMethods.end();
            ++iter) {
        DocumentRootConfig documentRootConfig = location.getDocumentRootConfig();
        switch (*iter) {
            case http::kMethodGet: { // NOLINT(bugprone-branch-clone)
                // http::IHandler *handler = new http::StaticFileHandler(documentRootConfig);
                // router_.addRoute(http::kMethodGet, location.getPath(), handler);
                break;
            }
            case http::kMethodDelete: {
                // http::IHandler *handler = new http::DeleteFileHandler(documentRootConfig);
                // router_.addRoute(http::kMethodDelete, location.getPath(), handler);
                break;
            }
            default:
                // do nothing
                break;
        }
    }
}

void VirtualServer::setupRouter() {
    LocationContextList locations = serverConfig_.getLocation();
    for (LocationContextList::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const LocationContext &location = *it;
        // if (location.getRedirect().isSome()) {
            // http::IHandler *handler = new http::RedirectHandler(location.getRedirect().unwrap());
            // router_.addRoute(location.getAllowedMethods(), location.getPath(), handler);
        // } else {
            this->registerHandlers(location);
        // }
    }

    router_.addMiddleware(new http::Logger());
    router_.addMiddleware(new http::ErrorPage(serverConfig_.getErrorPage()));
}
