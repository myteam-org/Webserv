#include <unistd.h>
#include <sys/stat.h>

#include "cgi.hpp"
#include "config/context/documentRootConfig.hpp"
#include "utils/path.hpp"
#include "utils/string.hpp"

namespace http {

namespace {

// utils    
bool initCheck(const Request& req, std::string* scriptPath,
               std::string* pathInfo);
bool checkIsUnderRoot(const std::string& root, const std::string& norm);
bool splitScriptAndPathInfo(const std::string& root, const std::string& norm,
                            std::string* scriptPath, std::string* pathInfo);
bool findScriptPrefix(const std::string& abs, std::string* scriptFsPath,
                      std::string* pathInfoSuffix);
bool isRegularFile(const std::string& file);
bool isExecutableFile(const std::string& file);
std::string fsToVirtual(const std::string& root, const std::string& fileSystem);

}  // namespace

// 本体
bool CgiHandler::isCgiTarget(const Request& req, std::string* scriptPath,
                             std::string* pathInfo) const {
    if (!initCheck(req, scriptPath, pathInfo)) {
        return false;
    }

    const std::string& root = docRootConfig_.getRoot();
    std::string rel = req.getPath();
    if (!rel.empty() && rel[0] == '/') {
        rel.erase(0, 1);
    }

    const std::string full = utils::joinPath(root, rel);
    const std::string norm = utils::normalizePath(full);

    if (!checkIsUnderRoot(root, norm)) {
        return false;
    }
    return splitScriptAndPathInfo(root, norm, scriptPath, pathInfo);
}

namespace {

bool initCheck(const Request& req, std::string* scriptPath,
               std::string* pathInfo) {
    if (scriptPath == NULL || pathInfo == NULL) {
        return false;
    }
    scriptPath->clear();
    pathInfo->clear();
    const http::HttpMethod method = req.getMethod();
    return method != kMethodGet && method != kMethodPost;
}

bool checkIsUnderRoot(const std::string& root, const std::string& norm) {
    if (root.empty()) {
        return false;
    }
    return utils::path::isPathUnderRoot(root, norm);
}

bool splitScriptAndPathInfo(const std::string& root, const std::string& norm,
                            std::string* scriptPath, std::string* pathInfo) {
    if (scriptPath == 0 || pathInfo == 0) {
        return false;
    }

    std::string scriptFsPath;
    std::string pathInfoSuffix;
    if (!findScriptPrefix(norm, &scriptFsPath, &pathInfoSuffix)) {
        return false;
    }
    // 拡張子チェック追加?
    if (!isExecutableFile(scriptFsPath)) {
        return false;
    }

    const std::string virt = fsToVirtual(root, scriptFsPath);
    if (virt.empty()) {
        return false;
    }

    *scriptPath = virt;
    *pathInfo = pathInfoSuffix;  // possible '/'start

    return true;
}

// 末尾からディレクトリを削っていき、最長一致の既存ファイルを見つける
// 見つかったら scriptFs=そのファイル, rest=残り（先頭'/'含む or 空）
bool findScriptPrefix(const std::string& abs, std::string* scriptFsPath,
                      std::string* pathInfoSuffix) {
    if (scriptFsPath == 0 || pathInfoSuffix == 0) {
        return false;
    }
    std::string cur = abs;
    while (true) {
        if (isRegularFile(cur)) {
            *scriptFsPath = cur;
            if (abs.size() > cur.size()) {
                *pathInfoSuffix = abs.substr(cur.size());
            } else {
                pathInfoSuffix->clear();
            }
            return true;
        }
        const std::string::size_type pos = cur.rfind('/');
        if (pos == std::string::npos || pos == 0) {
            return false;
        }
        cur = cur.substr(0, pos);
    }
}

// 正規ファイルか
bool isRegularFile(const std::string& file) {
    struct stat sta;

    if (stat(file.c_str(), &sta) != 0) {
        return false;
    }
    return S_ISREG(sta.st_mode);
}

// 実行可能ファイルか
bool isExecutableFile(const std::string& file) {
    if (!isRegularFile(file)) {
        return false;
    }
    return access(file.c_str(), X_OK) == 0;
}

std::string fsToVirtual(const std::string& root,
                        const std::string& fileSystem) {
    if (fileSystem.compare(0, root.size(), root) != 0) {
        return std::string();
    }
    std::string virt = fileSystem.substr(root.size());
    if (virt.empty() || virt[0] != '/') {
        virt.insert(virt.begin(), '/');
    }
    return virt;
}

}  // namespace

}  // namespace http
