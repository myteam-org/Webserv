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
#include "utils/path.hpp"
#include "utils/string.hpp"

namespace http {

UploadFileHandler::UploadFileHandler(const DocumentRootConfig& docRootConfig)
    : docRootConfig_(docRootConfig) {}

Either<IAction*, Response> UploadFileHandler::serve(const Request& request) {
    return Right(this->serveInternal(request));
}

Response UploadFileHandler::serveInternal(const Request& request) const {
    // Parser が decode + remove_dot_segments 済みのパスを返す前提
    const std::string& rel = request.getPath();

    const std::string& root = docRootConfig_.getRoot();
    const std::string joined = utils::joinPath(root, rel);

    // スラッシュ正規化（'\\' -> '/', '//' 圧縮）
    const std::string full = utils::path::normalizeSlashes(joined);

    // ルート配下チェック
    if (!isPathUnderRoot(full, root)) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }

    // 親ディレクトリの存在/権限
    const types::Result<types::Unit, HttpStatusCode> dirCheck =
        checkParentDir(full);
    if (dirCheck.isErr()) {
        return ResponseBuilder().status(dirCheck.unwrapErr()).build();
    }

    // 6) 書き込み
    return writeToFile(full, request.getBody());
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

// O_CLOEXEC は 「close-on-exec フラグをつけてファイルディスクリプタを開く」
// という意味で、exec()系システムコールを実行したときにそのファイルディスクリプタが
// 自動的に閉じられるようにする
Response UploadFileHandler::writeToFile(const std::string& path,
                                        const std::vector<char>& body) {
    int raw_fd;
#ifdef O_CLOEXEC
    raw_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
#else
    raw_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (raw_fd >= 0) {
        fcntl(raw_fd, F_SETFD, FD_CLOEXEC);
    }
#endif
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
