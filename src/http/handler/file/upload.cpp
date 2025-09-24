#include "http/handler/file/upload.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include "http/response/builder.hpp"
#include "io/base/FileDescriptor.hpp"
#include "utils/logger.hpp"
#include "utils/path.hpp"
#include "utils/string.hpp"

namespace http {


static std::string trim(const std::string& s) {
    std::string::size_type b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n'))
        ++b;
    std::string::size_type e = s.size();
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n'))
        --e;
    return s.substr(b, e - b);
}

static std::string sanitizeFilename(const std::string& orig) {
    if (orig.empty()) return "uploaded_file";
    std::string r;
    for (std::string::size_type i = 0; i < orig.size(); ++i) {
        char c = orig[i];
        if (c == '/' || c == '\\' || c == ':' || c == 0) c = '_';
        r += c;
    }
    // ".." を潰す
    if (r == "." || r == "..") r = "_";
    return r;
}

static std::string extractBoundary(const std::string& contentType) {
    // 例: multipart/form-data; boundary=----WebKitFormBoundaryABC123
    const std::string key = "boundary=";
    std::string::size_type pos = contentType.find(key);
    if (pos == std::string::npos) return "";
    pos += key.size();
    // 引用符で囲まれる場合対応
    if (pos < contentType.size() && (contentType[pos] == '"' || contentType[pos] == '\'')) {
        char quote = contentType[pos];
        ++pos;
        std::string::size_type end = contentType.find(quote, pos);
        if (end == std::string::npos) return "";
        return contentType.substr(pos, end - pos);
    }
    // 空白かセミコロンまで
    std::string::size_type end = pos;
    while (end < contentType.size() &&
           contentType[end] != ';' &&
           contentType[end] != ' ' &&
           contentType[end] != '\r' &&
           contentType[end] != '\n' &&
           contentType[end] != '\t') {
        ++end;
    }
    return contentType.substr(pos, end - pos);
}

// 最初のファイルパートのみ抜き出し
static bool parseFirstFilePart(const std::string& body,
                               const std::string& boundaryRaw,
                               std::string& outFilename,
                               std::string& outData) {
    outFilename = "";
    outData.clear();
    if (boundaryRaw.empty()) return false;

    const std::string boundary = "--" + boundaryRaw;
    std::string::size_type searchPos = 0;

    while (true) {
        // 境界線を探す
        std::string::size_type bPos = body.find(boundary, searchPos);
        if (bPos == std::string::npos) break;
        bPos += boundary.size();

        // 終端か？
        if (bPos + 2 <= body.size() && body.compare(bPos, 2, "--") == 0) {
            break; // 終了
        }

        // CRLF をスキップ
        if (bPos + 2 <= body.size() && body.compare(bPos, 2, "\r\n") == 0) {
            bPos += 2;
        } else if (bPos < body.size() && (body[bPos] == '\n')) {
            bPos += 1;
        }

        // ヘッダ終了位置を探す
        std::string::size_type headerEnd = body.find("\r\n\r\n", bPos);
        std::string::size_type headerSepLen = 4;
        if (headerEnd == std::string::npos) {
            // "\n\n" のみのパターン緩和
            headerEnd = body.find("\n\n", bPos);
            headerSepLen = 2;
        }
        if (headerEnd == std::string::npos) break;

        std::string headersBlock = body.substr(bPos, headerEnd - bPos);

        // パートのデータ開始
        std::string::size_type dataStart = headerEnd + headerSepLen;

        // 次の境界を探す
        std::string::size_type nextBoundary = body.find(boundary, dataStart);
        if (nextBoundary == std::string::npos) {
            // 終端が見つからない→不正 or 最後まで
            nextBoundary = body.size();
        }

        // パートの生データ範囲（末尾の CRLF を落としたい）
        std::string::size_type dataEnd = nextBoundary;
        // 末尾に "\r\n" があれば削る
        if (dataEnd >= 2 && body.compare(dataEnd - 2, 2, "\r\n") == 0) {
            dataEnd -= 2;
        } else if (dataEnd >= 1 && body[dataEnd - 1] == '\n') {
            dataEnd -= 1;
        }

        // ヘッダを行単位で解析
        std::string filename;
        {
            std::string::size_type lineStart = 0;
            while (lineStart < headersBlock.size()) {
                std::string::size_type lineEnd = headersBlock.find("\r\n", lineStart);
                std::string line;
                if (lineEnd == std::string::npos) {
                    line = headersBlock.substr(lineStart);
                    lineStart = headersBlock.size();
                } else {
                    line = headersBlock.substr(lineStart, lineEnd - lineStart);
                    lineStart = lineEnd + 2;
                }
                line = trim(line);
                if (line.empty()) continue;
                // Content-Disposition 行を探す
                std::string lower = line;
                for (std::string::size_type i = 0; i < lower.size(); ++i) {
                    if (lower[i] >= 'A' && lower[i] <= 'Z')
                        lower[i] = lower[i] - 'A' + 'a';
                }
                if (lower.find("content-disposition:") == 0) {
                    // filename="..."
                    const std::string key = "filename=\"";
                    std::string::size_type fPos = line.find(key);
                    if (fPos != std::string::npos) {
                        fPos += key.size();
                        std::string::size_type fEnd = line.find('"', fPos);
                        if (fEnd != std::string::npos) {
                            filename = line.substr(fPos, fEnd - fPos);
                        }
                    }
                }
            }
        }

        if (!filename.empty()) {
            outFilename = sanitizeFilename(filename);
            outData.assign(body, dataStart, dataEnd - dataStart);
            return true;
        }

        // 次へ
        searchPos = nextBoundary;
    }
    return false;
}

static std::string normalizePath(const std::string& path) {
    std::vector<std::string> stack;
    std::string::size_type i = 0, len = path.length();
    while (i < len) {
        // スラッシュをスキップ
        while (i < len && path[i] == '/') ++i;
        if (i >= len) break;
        // セグメント抽出
        std::string::size_type j = i;
        while (j < len && path[j] != '/') ++j;
        std::string seg = path.substr(i, j - i);
        if (seg == ".") {
            // 無視
        } else if (seg == "..") {
            if (!stack.empty()) stack.pop_back();
        } else if (!seg.empty()) {
            stack.push_back(seg);
        }
        i = j;
    }
    std::string result = "/";
    for (size_t k = 0; k < stack.size(); ++k) {
        if (k > 0) result += "/";
        result += stack[k];
    }
    return result;
}

static bool isPathUnderRootUnified(const std::string& rootPath,
                                   const std::string& targetPath) {
    std::string rootNorm = normalizePath(rootPath);
    std::string targetNorm = normalizePath(targetPath);

    // 末尾スラッシュ調整
    if (!rootNorm.empty() && rootNorm[rootNorm.length()-1] != '/')
        rootNorm += '/';
    if (!targetNorm.empty() && targetNorm[targetNorm.length()-1] != '/')
        targetNorm += '/';

    bool isValid = (targetNorm.find(rootNorm) == 0);
    return isValid;
}


UploadFileHandler::UploadFileHandler(const DocumentRootConfig& docRootConfig)
    : docRootConfig_(docRootConfig) {}

Either<IAction*, Response> UploadFileHandler::serve(const Request& request) {
    return Right(this->serveInternal(request));
}

Response UploadFileHandler::serveInternal(const Request& request) const {
    const std::string& rel = request.getPath(); // "/upload"

    if (!docRootConfig_.getEnableUpload()) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }

    const std::string root = utils::path::normalizeSlashes(docRootConfig_.getRoot());

    // ここでは /upload をファイル名に使わない（API エンドポイント扱い）
    // Content-Type 確認
    const types::Option<std::string> ctOpt = request.getHeader("Content-Type");
    if (ctOpt.isNone()) {
        return ResponseBuilder().status(kStatusBadRequest).build();
    }
    const std::string contentType = ctOpt.unwrap();
    if (contentType.find("multipart/form-data") == std::string::npos) {
        return ResponseBuilder().status(kStatusBadRequest).build();
    }
    std::string boundary = extractBoundary(contentType);
    if (boundary.empty()) {
        return ResponseBuilder().status(kStatusBadRequest).build();
    }

    // ボディ取得
    const std::vector<char>& bodyVec = request.getBody();
    std::string bodyStr;
    if (!bodyVec.empty())
        bodyStr.assign(&bodyVec[0], bodyVec.size());

    // パート解析
    std::string filename;
    std::string fileData;
    if (!parseFirstFilePart(bodyStr, boundary, filename, fileData)) {
        return ResponseBuilder().status(kStatusBadRequest).build();
    }

    // 保存パス構築
    std::string savePath = root;
    if (!savePath.empty() && savePath[savePath.size() - 1] != '/')
        savePath += '/';
    savePath += filename;

    // パス検証
    if (!isPathUnderRootUnified(root, savePath)) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }

    // 親ディレクトリ確認
    const types::Result<types::Unit, HttpStatusCode> dirCheck =
        checkParentDir(savePath);
    if (dirCheck.isErr()) {
        return ResponseBuilder().status(dirCheck.unwrapErr()).build();
    }

    // 書き込み
    int raw_fd;
#ifdef O_CLOEXEC
    raw_fd = open(savePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
#else
    raw_fd = open(savePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (raw_fd >= 0) {
        fcntl(raw_fd, F_SETFD, FD_CLOEXEC);
    }
#endif
    if (raw_fd < 0) {
        return ResponseBuilder().status(kStatusForbidden).build();
    }
    {
        FileDescriptor fd(raw_fd);
        const types::Option<int> fdOpt = fd.getFd();
        if (fdOpt.isNone()) {
            return ResponseBuilder().status(kStatusForbidden).build();
        }
        if (!fileData.empty()) {
            ssize_t written = write(fdOpt.unwrap(), fileData.data(), fileData.size());
            if (written < 0 || static_cast<size_t>(written) != fileData.size()) {
                return ResponseBuilder().status(kStatusInternalServerError).build();
            }
        }
    }

    return ResponseBuilder().status(kStatusCreated).build();
}

types::Result<types::Unit, HttpStatusCode>
UploadFileHandler::checkParentDir(const std::string& normalized) const {
    const std::string root = utils::path::normalizeSlashes(docRootConfig_.getRoot());
    if (!isPathUnderRootUnified(root, normalized)) {
        return ERR(kStatusForbidden);
    }
    std::string::size_type lastSlash = normalized.rfind('/');
    std::string dirPath =
        (lastSlash == std::string::npos) ? root : normalized.substr(0, lastSlash);

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

} // namespace http
