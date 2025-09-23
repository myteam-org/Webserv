#include "http/virtual_server.hpp"

#include "http/handler/router/builder.hpp"
#include "http/handler/file/cgi.hpp"
#include "http/handler/file/delete.hpp"
#include "http/handler/file/redirect.hpp"
#include "http/handler/file/static.hpp"
#include "http/handler/file/upload.hpp"
#include "http/status.hpp"
#include "http/handler/router/middleware/chain.hpp"
#include "http/handler/router/middleware/error_page.hpp"
#include "http/handler/router/middleware/logger.hpp"
#include "http/handler/router/router.hpp"

#include "utils/logger.hpp"

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
        LOG_INFO("registered uploadFileHandler");
    }
    if (allowed[DELETE] == ON) {
        routerBuilder.route(http::kMethodDelete, path, new http::DeleteFileHandler(docRoot));
    }
}

// LocationContext redirect_文字列がある場合はRedirectHandlerをnewする
// DocumentRootConfig cgi_ == ONの時は、CgiHandlerをnewする
// どちらでもない時はregisterHandlers()を呼んで該当のHandlerをnewする
void VirtualServer::setupRouter() {
    const http::HttpMethod httpMethods[] = {http::kMethodGet, http::kMethodPost, http::kMethodDelete};
    
    http::RouterBuilder routerBuilder;
    const LocationContextList locationContextList = serverConfig_.getLocation();
    for (LocationContextList::const_iterator locationIterator =
             locationContextList.begin();
             locationIterator != locationContextList.end(); ++locationIterator) {
        const LocationContext &locationContext = *locationIterator;
        const std::string &path = locationContext.getPath();
        const DocumentRootConfig &docRoot = locationContext.getDocumentRootConfig();
        const OnOff *allowedMethod = locationContext.getAllowedMethod();
        for (size_t i = 0; i < sizeof(httpMethods)/sizeof(httpMethods[0]); ++i) {
            const std::string &redirect = locationContext.getRedirect();
            if (!redirect.empty() && allowedMethod[httpMethods[i]] == ON) {
                routerBuilder.route(httpMethods[i], path, new http::RedirectHandler(redirect));
            } else if (docRoot.getCgiExtensions() == ON && allowedMethod[httpMethods[i]] == ON) {
                //     routerBuilder.route(allowedMethods[i], path, new http::CgiHandler(docRoot));
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
