#include "http/response/builder.hpp"
#include "http/handler/router/builder.hpp"
#include "http/handler/file/static.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cerrno>
#include <cstring>
#include "utils/string.hpp"
#include "utils/logger.hpp" // ログ用ヘッダ追加

namespace http {

namespace {
    Response buildFileResponse(const struct stat &fileStatus, const std::string &filePath) {
        if (!S_ISREG(fileStatus.st_mode)) {
            LOG_ERROR("ファイルではありません: " + filePath);
            return ResponseBuilder().status(kStatusForbidden).build();
        }
        LOG_INFO("通常ファイルを返します: " + filePath);
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
    LOG_INFO("StaticFileHandler初期化: root=" + documentRootConfig_.getRoot());
}

Either<IAction *, Response> StaticFileHandler::serve(const Request &requestContext) {
    LOG_INFO("serve関数呼び出し: target=" + requestContext.getRequestTarget());
    return Right(this->serveInternal(requestContext));
}

types::Result<std::string, HttpStatusCode>
StaticFileHandler::makeDirectoryListingHtml(const std::string &rootPath, const std::string &targetPath) {
    const std::string fullPath = rootPath + targetPath;
    LOG_INFO("ディレクトリlisting生成: fullPath=" + fullPath + " target=" + targetPath);
    DIR *directory = opendir(fullPath.c_str());
    if (directory == NULL) {
        LOG_ERROR("ディレクトリを開けません: " + fullPath + " errno=" + utils::toString(errno) + " : " + strerror(errno));
        return ERR(kStatusInternalServerError);
    }
    return OK(createDirectoryListingHtml(directory, targetPath));
}

Response StaticFileHandler::directoryListing(const std::string &rootPath, const std::string &targetPath) {
    LOG_INFO("directoryListing呼び出し: root=" + rootPath + " target=" + targetPath);
    const types::Result<std::string, HttpStatusCode> htmlResult = makeDirectoryListingHtml(rootPath, targetPath);
    if (htmlResult.isErr()) {
        LOG_ERROR("ディレクトリリストHTML生成失敗: root=" + rootPath + " target=" + targetPath);
        return ResponseBuilder().status(htmlResult.unwrapErr()).build();
    }
    LOG_INFO("ディレクトリリストHTML生成成功: root=" + rootPath + " target=" + targetPath);
    return ResponseBuilder().html(htmlResult.unwrap()).build();
}

Response StaticFileHandler::handleDirectory(const Request &request, const std::string &directoryPath) const {
    LOG_INFO("handleDirectory呼び出し: requestTarget=" + request.getRequestTarget() + " directoryPath=" + directoryPath);

    if (!utils::endsWith(request.getRequestTarget(), "/")) {
        LOG_INFO("末尾/なし。リダイレクト: " + request.getRequestTarget() + "/");
        return ResponseBuilder().redirect(request.getRequestTarget() + "/").build();
    }

    const std::string indexFilePath = directoryPath + documentRootConfig_.getIndex();
    LOG_INFO("indexファイルパス: " + indexFilePath);
    struct stat indexFileStatus = {};
    if (stat(indexFilePath.c_str(), &indexFileStatus) != -1) {
        LOG_INFO("indexファイルあり: " + indexFilePath);
        return buildFileResponse(indexFileStatus, indexFilePath);
    }

    LOG_ERROR("indexファイルが見つかりません: " + indexFilePath + " errno=" + utils::toString(errno) + " : " + strerror(errno));
    LOG_INFO("autoindex設定: " + (documentRootConfig_.isAutoindexEnabled() ? "ON" : "OFF"));

    if (errno == ENOENT && documentRootConfig_.isAutoindexEnabled()) {
        LOG_INFO("autoindex有効。ディレクトリリストを表示します: " + directoryPath);
        return directoryListing(documentRootConfig_.getRoot(), request.getRequestTarget());
    }

    if (errno == ENOENT || errno == EACCES) {
        LOG_ERROR("ディレクトリアクセス禁止: " + indexFilePath + " errno=" + utils::toString(errno) + " : " + strerror(errno));
        return ResponseBuilder().status(kStatusForbidden).build();
    }

    LOG_ERROR("予期しないディレクトリエラー: " + indexFilePath + " errno=" + utils::toString(errno) + " : " + strerror(errno));
    return ResponseBuilder().status(kStatusInternalServerError).build();
}

Response StaticFileHandler::serveInternal(const Request &request) const {
    // locationのpathが /upload の場合、/upload へのリクエストは ./www/user_uploads/ ディレクトリそのものを指すべき
    // そのためパス構成を明示的にログ
    LOG_INFO("serveInternal呼び出し: requestTarget=" + request.getRequestTarget());
    LOG_INFO("documentRoot=" + documentRootConfig_.getRoot());
    LOG_INFO("locationPath=" + documentRootConfig_.getLocationPath());

    std::string filePath = documentRootConfig_.getRoot();
    if (request.getRequestTarget().size() > documentRootConfig_.getLocationPath().size()) {
        // 例: /upload/foo.txt → "foo.txt" を追加
        filePath += request.getRequestTarget().substr(documentRootConfig_.getLocationPath().size());
    }
    LOG_INFO("最終的にstatするパス: " + filePath);

    struct stat fileStatus = {};
    if (stat(filePath.c_str(), &fileStatus) == -1) {
        LOG_ERROR("ファイルstat失敗: " + filePath + " errno=" + utils::toString(errno) + " : " + strerror(errno));
        if (errno == ENOENT) {
            return ResponseBuilder().status(kStatusNotFound).build();
        }
        if (errno == EACCES) {
            return ResponseBuilder().status(kStatusForbidden).build();
        }
        return ResponseBuilder().status(kStatusInternalServerError).build();
    }

    if (S_ISDIR(fileStatus.st_mode)) {
        LOG_INFO("ディレクトリです。handleDirectoryへ: " + filePath);
        return handleDirectory(request, filePath);
    }
    LOG_INFO("通常ファイルです。buildFileResponseへ: " + filePath);

    return buildFileResponse(fileStatus, filePath);
}

}  // namespace http
