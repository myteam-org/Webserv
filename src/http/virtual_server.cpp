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

// ---- Hostによる仮想サーバ選択 ----
// matchesHostはhost_は使わず、server_namesのみで判定
bool VirtualServer::matchesHost(const std::string &host) const {
    std::string hostOnly = host;
    std::string::size_type colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        hostOnly = host.substr(0, colonPos);
    }
    // server_namesのみ
    const std::vector<std::string> &names = serverConfig_.getServerNames();
    for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it) {
        if (*it == hostOnly) {
            return true;
        }
    }
    return false;
}

VirtualServer* VirtualServer::findByHost(const std::vector<VirtualServer*>& servers, const std::string& host) {
    for (size_t i = 0; i < servers.size(); ++i) {
        if (servers[i]->matchesHost(host)) {
            return servers[i];
        }
    }
    return NULL;
}

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

http::Router &VirtualServer::getRouter() {
    return *router_;
}

// void VirtualServer::registerHandlers(http::RouterBuilder &routerBuilder,
//                                      const LocationContext &locationContext) {
//     const DocumentRootConfig &docRoot = locationContext.getDocumentRootConfig();
//     const std::string &path = locationContext.getPath();
//     const OnOff *allowed = locationContext.getAllowedMethod();

//     if (allowed[GET] == ON) {
//         routerBuilder.route(http::kMethodGet, path, new http::StaticFileHandler(docRoot));
//     }
//     if (allowed[POST] == ON) {
//         routerBuilder.route(http::kMethodPost, path, new http::UploadFileHandler(docRoot));
//     }
//     if (allowed[DELETE] == ON) {
//         routerBuilder.route(http::kMethodDelete, path, new http::DeleteFileHandler(docRoot));
//     }
// }

// LocationContext redirect_文字列がある場合はRedirectHandlerをnewする
// DocumentRootConfig cgi_ == ONの時は、CgiHandlerをnewする
// どちらでもない時はregisterHandlers()を呼んで該当のHandlerをnewする
void VirtualServer::setupRouter() {
    http::RouterBuilder routerBuilder;
    const LocationContextList locationContextList = serverConfig_.getLocation();

    for (LocationContextList::const_iterator it = locationContextList.begin();
         it != locationContextList.end(); ++it) {

        const LocationContext &loc = *it;
        const std::string &path = loc.getPath();
        const DocumentRootConfig &docRoot = loc.getDocumentRootConfig();
        const OnOff *allowed = loc.getAllowedMethod();
        const std::string &redirect = loc.getRedirect();

        // Redirect 優先 (任意: メソッド別に許可チェック)
        if (!redirect.empty()) {
            if (allowed[GET] == ON)
                routerBuilder.route(http::kMethodGet, path,
                                    new http::RedirectHandler(redirect));
            if (allowed[POST] == ON)
                routerBuilder.route(http::kMethodPost, path,
                                    new http::RedirectHandler(redirect));
            if (allowed[DELETE] == ON)
                routerBuilder.route(http::kMethodDelete, path,
                                    new http::RedirectHandler(redirect));
            continue;
        }

        // CGI 対応 (いまはコメントアウトされていたので保留)
        bool cgiOn = (docRoot.getCgiExtensions() == ON);

        if (allowed[GET] == ON) {
            if (cgiOn) {
                routerBuilder.route(http::kMethodGet, path, new http::CgiHandler(docRoot));
                // routerBuilder.route(http::kMethodGet, path, new http::CgiHandler(docRoot, path));
            } else {
                routerBuilder.route(http::kMethodGet, path,
                                    new http::StaticFileHandler(docRoot));
            }
        }
        if (allowed[POST] == ON) {
            if (cgiOn) {
                routerBuilder.route(http::kMethodGet, path, new http::CgiHandler(docRoot));
                // routerBuilder.route(http::kMethodPost, path, new http::CgiHandler(docRoot, path));
            } else {
                routerBuilder.route(http::kMethodPost, path,
                                    new http::UploadFileHandler(docRoot));
            }
        }
        if (allowed[DELETE] == ON) {
            routerBuilder.route(http::kMethodDelete, path,
                                new http::DeleteFileHandler(docRoot, path));
        }
    }

    routerBuilder.middleware(new http::Logger());
    routerBuilder.middleware(new http::ErrorPage(serverConfig_.getErrorPage()));

    if (router_ != NULL) {
        delete router_;
    }
    router_ = routerBuilder.build();
}
