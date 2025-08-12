#include "virtual_server.hpp"

#include "handler/router/builder.hpp"
#include "http/handler/file/cgi.hpp"
#include "http/handler/file/delete.hpp"
#include "http/handler/file/redirect.hpp"
#include "http/handler/file/static.hpp"
#include "http/handler/file/upload.hpp"
#include "http/status.hpp"
#include "middleware/chain.hpp"
#include "router/middleware/error_page.hpp"
#include "router/middleware/logger.hpp"
#include "router/router.hpp"

VirtualServer::VirtualServer(const ServerContext &serverConfig,
                             const std::string &bindAddress)
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

http::Router &VirtualServer::getRouter() { return *router_; }

// ビルド時に “各 location を順に登録する” 関数
void VirtualServer::registerHandlers(http::RouterBuilder &routerBuilder,
                                     const LocationContext &locationContext) {
    const DocumentRootConfig &docRoot = locationContext.getDocumentRootConfig();
    const std::string& path = locationContext.getPath();
    const OnOff *allowed = locationContext.getAllowedMethod();
    
    if (allowed[GET] == ON) {
        routerBuilder.route(http::kMethodGet, path,
                            new http::StaticFileHandler(docRoot));
    }
    if (allowed[POST] == ON) {
        routerBuilder.route(http::kMethodPost, path,
                            new http::UploadFileHandler(docRoot));
    }
    if (allowed[DELETE] == ON) {
        routerBuilder.route(http::kMethodDelete, path,
                            new http::DeleteFileHandler(docRoot));
    }
    // routerBuilder.route(http::kMethodPost, path, new http::CgiHandler(docRoot));
}

void VirtualServer::setupRouter() {
    http::RouterBuilder routerBuilder;
    const LocationContextList locationContextList = serverConfig_.getLocation();

    for (LocationContextList::const_iterator locationIterator = locationContextList.begin();
         locationIterator != locationContextList.end(); ++locationIterator) {
        const LocationContext &locationContext = *locationIterator;
        if (!locationContext.getRedirect().empty()) {
            http::IHandler *redirectHandler =
                new http::RedirectHandler(locationContext.getRedirect());
            routerBuilder.route(locationContext.getAllowedMethods(),
                                locationContext.getPath(), redirectHandler);
        } else {
            registerHandlers(routerBuilder, locationContext);
        }
    }

    routerBuilder.middleware(new http::Logger());
    const ErrorPageMap errorPages = serverConfig_.getErrorPage();
    routerBuilder.middleware(new http::ErrorPage(errorPages));

    if (router_ != NULL) {
        delete router_;
    }
    router_ = routerBuilder.build();
}
