#pragma once

#include "handler.hpp"
#include "config/config.hpp"
#include "result.hpp"

namespace http {

class StaticFileHandler : public IHandler {
public:
    explicit StaticFileHandler(const DocumentRootConfig &documentRootConfig);

    Either<IAction *, Response> serve(const Request &requestContext);

private:
    const DocumentRootConfig documentRootConfig_;

    static types::Result<std::string, HttpStatusCode> makeDirectoryListingHtml(
        const std::string &rootPath, const std::string &targetPath);

    static Response directoryListing(
        const std::string &rootPath, const std::string &targetPath);

    Response handleDirectory(
        const Request &request, const std::string &directoryPath) const;

    Response serveInternal(const Request &request) const;
};

}  // namespace http
