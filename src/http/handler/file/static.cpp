#include "http/response/builder.hpp"
#include "http/handler/router/builder.hpp"
#include "http/handler/file/static.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cerrno>
#include <cstring>
#include "utils/string.hpp"


namespace http {

namespace {
    Response buildFileResponse(const struct stat &fileStatus, const std::string &filePath) {
        if (!S_ISREG(fileStatus.st_mode)) {
            return ResponseBuilder().status(kStatusForbidden).build();
        }
        return ResponseBuilder().file(filePath).build();
    }

    void appendDirectoryEntry(std::string &htmlContent, const dirent *directoryEntry) {
        const std::string entryName = directoryEntry->d_name;
        if (entryName == ".") {
            return;
        }
        htmlContent += "<li><a href=\"";
        htmlContent += entryName;
        if (directoryEntry->d_type == DT_DIR) {
            htmlContent += "/";
        }
        htmlContent += "\">";
        htmlContent += entryName;
        if (directoryEntry->d_type == DT_DIR) {
            htmlContent += "/";
        }
        htmlContent += "</a></li>";
    }

    std::string createDirectoryListingHtml(DIR *directory, const std::string &targetPath) {
        const std::string indexTitle = "Index of " + targetPath;
        const int initialCapacity = 1024;
        std::string htmlContent;
        htmlContent.reserve(initialCapacity);

        htmlContent += "<!DOCTYPE html><html>";
        htmlContent += "<head><title>" + indexTitle + "</title></head>";
        htmlContent += "<body><h1>" + indexTitle + "</h1><hr><ul>";

        const dirent *directoryEntry;
        while ((directoryEntry = readdir(directory)) != NULL) {
            appendDirectoryEntry(htmlContent, directoryEntry);
        }
        closedir(directory);

        htmlContent += "</ul><hr></body></html>";
        return htmlContent;
    }
} // namespace

StaticFileHandler::StaticFileHandler(const DocumentRootConfig &documentRootConfig)
    : documentRootConfig_(documentRootConfig) {
}

Either<IAction *, Response> StaticFileHandler::serve(const Request &requestContext) {
    LOG_INFO("StaticFileHandler is invoked");
    return Right(this->serveInternal(requestContext));
}

types::Result<std::string, HttpStatusCode>
StaticFileHandler::makeDirectoryListingHtml(const std::string &rootPath, const std::string &targetPath) {
    const std::string fullPath = rootPath + targetPath;
    DIR *directory = opendir(fullPath.c_str());
    if (directory == NULL) {
        return ERR(kStatusInternalServerError);
    }
    return OK(createDirectoryListingHtml(directory, targetPath));
}

Response StaticFileHandler::directoryListing(const std::string &rootPath, const std::string &targetPath) {
    const types::Result<std::string, HttpStatusCode> htmlResult = makeDirectoryListingHtml(rootPath, targetPath);
    if (htmlResult.isErr()) {
        return ResponseBuilder().status(htmlResult.unwrapErr()).build();
    }
    return ResponseBuilder().html(htmlResult.unwrap()).build();
}

Response StaticFileHandler::handleDirectory(const Request &request, const std::string &directoryPath) const {

    if (!utils::endsWith(request.getRequestTarget(), "/")) {
        return ResponseBuilder().redirect(request.getRequestTarget() + "/").build();
    }

    const std::string indexFilePath = directoryPath + documentRootConfig_.getIndex();
    struct stat indexFileStatus = {};
    if (stat(indexFilePath.c_str(), &indexFileStatus) != -1) {
        return buildFileResponse(indexFileStatus, indexFilePath);
    }

    if (errno == ENOENT && documentRootConfig_.isAutoindexEnabled()) {
        return directoryListing(documentRootConfig_.getRoot(), request.getRequestTarget());
    }

    if (errno == ENOENT || errno == EACCES) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }

    return ResponseBuilder().status(kStatusInternalServerError).build();
}

Response StaticFileHandler::serveInternal(const Request &request) const {
    const std::string filePath = documentRootConfig_.getRoot() + request.getRequestTarget();

    struct stat fileStatus = {};
    if (stat(filePath.c_str(), &fileStatus) == -1) {
        if (errno == ENOENT) {
            return ResponseBuilder().status(kStatusNotFound).build();
        }
        if (errno == EACCES) {
            return ResponseBuilder().status(kStatusForbidden).build();
        }
        return ResponseBuilder().status(kStatusInternalServerError).build();
    }

    if (S_ISDIR(fileStatus.st_mode)) {
        return handleDirectory(request, filePath);
    }

    return buildFileResponse(fileStatus, filePath);
}

}  // namespace http
