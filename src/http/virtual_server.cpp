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

void VirtualServer::registerHandlers(http::RouterBuilder &routerBuilder,
                                     const LocationContext &locationContext) {
    const DocumentRootConfig &docRoot = locationContext.getDocumentRootConfig();
    const std::string &path = locationContext.getPath();
    const OnOff *allowed = locationContext.getAllowedMethod();

    if (allowed[GET] == ON) {
        routerBuilder.route(http::kMethodGet, path, new http::StaticFileHandler(docRoot));
    }
    if (allowed[POST] == ON) {
        routerBuilder.route(http::kMethodPost, path, new http::UploadFileHandler(docRoot));
    }
    if (allowed[DELETE] == ON) {
        routerBuilder.route(http::kMethodDelete, path, new http::DeleteFileHandler(docRoot));
    }
}

void VirtualServer::setupRouter() {
    http::RouterBuilder routerBuilder;
    const LocationContextList locationContextList = serverConfig_.getLocation();
    for (LocationContextList::const_iterator locationIterator =
             locationContextList.begin();
             locationIterator != locationContextList.end(); ++locationIterator) {
        const LocationContext &locationContext = *locationIterator;
        const std::string &path = locationContext.getPath();
        const DocumentRootConfig &docRoot = locationContext.getDocumentRootConfig();
        const OnOff *allowedMethod = locationContext.getAllowedMethod();
        http::HttpMethod method;
        for (method = http::kMethodGet; method <= http::kMethodDelete;
             method = static_cast<http::HttpMethod>(method + 1)) {
            const std::string &redirect = locationContext.getRedirect();
            if (!redirect.empty()) {
                routerBuilder.route(method, path, new http::RedirectHandler(redirect));
            // } else if (!docRoot.getCgiExtensions() == ON) {
            //     routerBuilder.route(method, path, new http::CgiHandler(docRoot));
            } else {
                registerHandlers(routerBuilder, locationContext);
            }
        }
    }
    routerBuilder.middleware(new http::Logger());
    routerBuilder.middleware(new http::ErrorPage(serverConfig_.getErrorPage()));
    if (router_ != NULL) {
        delete router_;
    }
    router_ = routerBuilder.build();
}
