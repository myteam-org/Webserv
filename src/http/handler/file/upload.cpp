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
#include "io/base/FileDescriptor.hpp"
#include "utils/string.hpp"

namespace http {

UploadFileHandler::UploadFileHandler(const DocumentRootConfig& docRootConfig)
    : docRootConfig_(docRootConfig) {}

Either<IAction*, Response> UploadFileHandler::serve(const Request& request) {
    return Right(this->serveInternal(request));
}

Response UploadFileHandler::serveInternal(const Request& request) const {
    std::string normalized;

    if (!decodeAndNomalizePath(request.getPath(), normalized)) {
        return ResponseBuilder().status(kStatusBadRequest).build();
    }

    const types::Result<types::Unit, HttpStatusCode> dirCheck =
        checkParentDir(normalized);

    if (dirCheck.isErr()) {
        return ResponseBuilder().status(dirCheck.unwrapErr()).build();
    }

    return writeToFile(normalized, request.getBody());
}

// パストラバーサル攻撃に対する検証
bool UploadFileHandler::decodeAndNomalizePath(const std::string& rawPath,
                                              std::string& normalized) const {
    std::string decoded;

    if (!UploadFileHandler::urlDecodeStrict(rawPath, decoded)) {
        return false;
    }
    while (!decoded.empty() && decoded[0] == '/') {
        decoded.erase(0, 1);
    }
    for (size_t i = 0; i < decoded.size(); ++i) {
        if (decoded[i] == '\\') {
            decoded[i] = '/';
        }
    }
    normalized = utils::normalizePath(
        utils::joinPath(docRootConfig_.getRoot(), decoded));
    return true;
}

bool UploadFileHandler::urlDecodeStrict(const std::string& src,
                                        std::string& out) {
    out.clear();
    for (std::size_t i = 0; i < src.size(); ++i) {
        const char chr = src[i];
        if (chr == '%') {
            if (i + 2 >= src.size() ||
                !std::isxdigit(static_cast<unsigned char>(src[i + 1])) ||
                !std::isxdigit(static_cast<unsigned char>(src[i + 2]))) {
                return false;
            }
            const int high = std::isdigit(src[i + 1])
                                 ? src[i + 1] - '0'
                                 : (std::toupper(src[i + 1]) - 'A' + 10);
            const int low = std::isdigit(src[i + 2])
                                ? src[i + 2] - '0'
                                : (std::toupper(src[i + 2]) - 'A' + 10);
            const char decoded = static_cast<char>((high << 4) | low);
            if (static_cast<unsigned char>(decoded) < kAsciiSpace &&
                decoded != '\t') {
                return false;
            }
            out.push_back(decoded);
            i += 2;
        } else {
            out.push_back(chr);
        }
    }
    return true;
}

types::Result<types::Unit, HttpStatusCode> UploadFileHandler::checkParentDir(
    const std::string& normalized) const {
    const std::string& root = docRootConfig_.getRoot();
    if (!isPathUnderRoot(normalized, root)) {
        return ERR(kStatusForbidden);
    }

    const std::string::size_type lastSlash = normalized.rfind('/');
    const std::string dirPath = (lastSlash == std::string::npos)
                                    ? root
                                    : normalized.substr(0, lastSlash);
    struct stat sta;
    if (stat(dirPath.c_str(), &sta) != 0) {
        return ERR(kStatusNotFound);
    }
    if (!S_ISDIR(sta.st_mode)) {
        return ERR(kStatusForbidden);
    }
    if (access(dirPath.c_str(), W_OK | X_OK) != 0) {
        return ERR(kStatusForbidden);
    }
    return OK(types::Unit());
}

Response UploadFileHandler::writeToFile(const std::string& path,
                                        const std::vector<char>& body) {
    const int raw_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (raw_fd < 0) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }
    const FileDescriptor fd(raw_fd);
    const types::Option<int> fdOpt = fd.getFd();
    if (fdOpt.isNone()) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }
    if (body.empty()) {
        return ResponseBuilder().status(kStatusCreated).build();
    }
    const ssize_t written = write(fdOpt.unwrap(), body.data(), body.size());
    if (written < 0 || static_cast<size_t>(written) != body.size()) {
        return ResponseBuilder().status(kStatusInternalServerError).build();
    }

    return ResponseBuilder().status(kStatusCreated).build();
}

bool UploadFileHandler::isPathUnderRoot(const std::string& path,
                                        const std::string& root) {
    if (root.empty()) {
        return false;
    }
    return path.compare(0, root.size(), root) == 0 &&
           (path.size() == root.size() || path[root.size()] == '/');
}

}  // namespace http
