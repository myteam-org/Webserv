#include "virtual_server.hpp"

#include "handler/router/builder.hpp"
#include "middleware/chain.hpp"
#include "router/router.hpp"
#include "router/middleware/logger.hpp"
#include "router/middleware/error_page.hpp"
#include "http/handler/file/redirect.hpp"
#include "http/handler/file/static.hpp"
#include "http/handler/file/upload.hpp"
#include "http/handler/file/delete.hpp"
#include "http/handler/file/cgi.hpp"
#include "http/status.hpp"

namespace {
ErrorPageMap MergeErrorPages(const std::vector<std::map<int, std::string> >& vec) {
    ErrorPageMap out;

    for (std::vector<std::map<int, std::string> >::const_iterator it = vec.begin(); it != vec.end(); ++it) {
        for (std::map<int, std::string>::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
            out[static_cast<http::HttpStatusCode>(jt->first)] = jt->second; 
        }
    }
    return out;
}
} // namespace

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
    const DocumentRootConfig& docRoot = locationContext.getDocumentRootConfig();
    const std::string& path = locationContext.getPath();
    const OnOff* allowed = locationContext.getAllowedMethod();

    const bool isUploadLocation = (path == "/upload");
    const bool cgiEnabled = (docRoot.getCgiExtensions() == ON);

    if (isUploadLocation && allowed[POST] == ON) {
        routerBuilder.route(http::kMethodPost, path, new http::UploadFileHandler(docRoot));
    }
    if (!isUploadLocation && cgiEnabled) {
        if (allowed[GET] == ON) {
            // routerBuilder.route(http::kMethodGet, path, new http::CgiHandler(docRoot));
        }
        if (allowed[POST] == ON) {
            // routerBuilder.route(http::kMethodPost, path, new http::CgiHandler(docRoot));
        }
        return ;
    }

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
    const std::vector<LocationContext> &locations = serverConfig_.getLocation();

    for (LocationContextList::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        const LocationContext &locationContext = *it;
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
    const ErrorPageMap merged = MergeErrorPages(serverConfig_.getErrorPage());
    routerBuilder.middleware(new
    http::ErrorPage(merged));

    if (router_ != NULL) {
        delete router_;
    }
    router_ = routerBuilder.build();
}
