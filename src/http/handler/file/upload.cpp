#include "upload.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "http/response/builder.hpp"
#include "utils/string.hpp"

namespace http {

UploadFileHandler::UploadFileHandler(const DocumentRootConfig& docRootConfig)
    : docRootConfig_(docRootConfig) {}

Either<IAction*, Response> UploadFileHandler::serve(const Request& request) {
    return Right(this->serveInternal(request));
}

Response UploadFileHandler::serveInternal(const Request& request) const {
    const std::string& root = docRootConfig_.getRoot();
    std::string userPath = request.getPath();

    if (!userPath.empty() && userPath[0] == '/') {
        userPath = userPath.substr(1);
    }
    if (!isValidUserPath(userPath)) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }

    const std::string fullPath = utils::joinPath(root, userPath);
    const std::string normalized = utils::normalizePath(fullPath);

    if (!isPathUnderRoot(normalized, root)) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }

    const std::string::size_type lastSlash = normalized.rfind('/');
    
    if (lastSlash != std::string::npos) {
        const std::string dirPath = normalized.substr(0, lastSlash);
        struct stat sta;
        if (stat(dirPath.c_str(), &sta) != 0) {
            return ResponseBuilder().status(kStatusNotFound).build();
        }
        if (!S_ISDIR(sta.st_mode)) {
            return ResponseBuilder().status(kStatusForbidden).build();
        }
    }
    return writeToFile(normalized, request.getBody());
}

bool UploadFileHandler::isValidUserPath(const std::string& path) {
    return !path.empty() && path.find("..") == std::string::npos;
}

bool UploadFileHandler::isPathUnderRoot(const std::string& path,
                                        const std::string& root) {
    return path.compare(0, root.size(), root) == 0 &&
           (path.size() == root.size() || path[root.size()] == '/');
}

Response UploadFileHandler::writeToFile(const std::string& path,
                                        const std::vector<char>& body) {
    const int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }
    const ssize_t written = write(fd, body.data(), body.size());
    close(fd);
    if (written < 0 || static_cast<size_t>(written) != body.size()) {
        return ResponseBuilder().status(kStatusInternalServerError).build();
    }

    return ResponseBuilder().status(kStatusCreated).build();
}

}  // namespace http
