#include "virtual_server.hpp"
#include "router/router.hpp"
#include "handler/router/builder.hpp"
#include "middleware/chain.hpp"

VirtualServer::VirtualServer(const ServerContext &serverConfig, const std::string &bindAddress)
    : serverConfig_(serverConfig), bindAddress_(bindAddress), router_(NULL) {
    this->setupRouter();
}

VirtualServer::~VirtualServer() {
    if (router_ != NULL) {
        delete router_;
    }
}

const ServerContext &VirtualServer::getServerConfig() const {
    return serverConfig_;
}

http::Router &VirtualServer::getRouter() {
    return *router_;
}

void VirtualServer::registerHandlers(http::RouterBuilder &routerBuilder, const LocationContext &locationContext) {
    (void)routerBuilder; // 未使用パラメータ警告対策
    const std::vector<http::HttpMethod> allowedHttpMethods = locationContext.getAllowedMethods();
    for (std::vector<http::HttpMethod>::const_iterator methodIterator = allowedHttpMethods.begin();
         methodIterator != allowedHttpMethods.end();
         ++methodIterator) {
        const DocumentRootConfig documentRootConfig = locationContext.getDocumentRootConfig(); // NOLINT
        switch (*methodIterator) {
            case http::kMethodGet: { // NOLINT(bugprone-branch-clone)
                // http::IHandler *getHandler = new http::StaticFileHandler(documentRootConfig);
                // routerBuilder.route(http::kMethodGet, locationContext.getPath(), getHandler);
                break;
            }
            case http::kMethodDelete: {
                // http::IHandler *deleteHandler = new http::DeleteFileHandler(documentRootConfig);
                // routerBuilder.route(http::kMethodDelete, locationContext.getPath(), deleteHandler);
                break;
            }
            default:
                // 何もしない
                break;
        }
    }
}

void VirtualServer::setupRouter() {
    http::RouterBuilder routerBuilder;
    const LocationContextList locationContextList = serverConfig_.getLocation();
    for (LocationContextList::const_iterator locationIterator = locationContextList.begin();
         locationIterator != locationContextList.end();
         ++locationIterator) {
        const LocationContext &locationContext = *locationIterator;
        // if (locationContext.getRedirect().isSome()) {
            // http::IHandler *redirectHandler = new http::RedirectHandler(locationContext.getRedirect().unwrap());
            // routerBuilder.route(locationContext.getAllowedMethods(), locationContext.getPath(), redirectHandler);
        // } else {
            registerHandlers(routerBuilder, locationContext);
        // }
    }

    // routerBuilder.middleware(new http::Logger());
    // routerBuilder.middleware(new http::ErrorPage(serverConfig_.getErrorPage()));

    if (router_ != NULL) {
        delete router_;
    }
    router_ = routerBuilder.build();
}
