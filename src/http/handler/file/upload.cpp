#include "upload.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
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
    const std::string fullPath = utils::joinPath(docRootConfig_.getRoot(),  request.getPath());

    // check the exsistence of the directory
    const std::string::size_type lastSlash = fullPath.rfind('/');
    if (lastSlash != std::string::npos) {
        const std::string dirPath = fullPath.substr(0, lastSlash);
        struct stat sta;
        if (stat(dirPath.c_str(), &sta) != 0) {
            return ResponseBuilder().status(kStatusNotFound).build();
        }
        if (!S_ISDIR(sta.st_mode)) {
            return ResponseBuilder().status(kStatusForbidden).build();
        }
    }
    // write the body to a file
    const int fd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }
    const std::vector<char>& body = request.getBody();
    const ssize_t written = write(fd, &body[0], body.size());
    close(fd);

    if (written < 0 || static_cast<size_t>(written) != body.size()) {
        return ResponseBuilder().status(kStatusInternalServerError).build();
    }
    return ResponseBuilder().status(kStatusCreated).build();
}

}  // namespace http
