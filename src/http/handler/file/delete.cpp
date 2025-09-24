#include "http/handler/file/delete.hpp"
#include "http/response/builder.hpp"
#include "utils/logger.hpp"
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <string>

namespace http {
    DeleteFileHandler::DeleteFileHandler(const DocumentRootConfig &docRootConfig)
        : docRootConfig_(docRootConfig) {}

    Either<IAction *, Response> DeleteFileHandler::serve(const Request &request) {
        return Right(this->serveInternal(request));
    }

    Response DeleteFileHandler::serveInternal(const Request &req) const {
        LOG_DEBUG("docRootConfig_.getRoot(): " + docRootConfig_.getRoot());
        LOG_DEBUG("req.getRequestTarget(): " + req.getRequestTarget());

        // 先頭が /upload/ なら除去
        std::string target = req.getRequestTarget();
        const std::string upload_prefix = "/upload/";
        if (target.compare(0, upload_prefix.length(), upload_prefix) == 0) {
            target = target.substr(upload_prefix.length());
        }
        LOG_DEBUG("修正後のリクエストターゲット: " + target);

        // パスを構築
        const std::string path = docRootConfig_.getRoot() + '/' + target;
        LOG_DEBUG("DELETEリクエスト対象パス: " + path);

        struct stat buf;
        memset(&buf, 0, sizeof(buf));

        if (stat(path.c_str(), &buf) == -1) {
            LOG_DEBUG("stat失敗: errno=" + std::string(strerror(errno)));
            if (errno == ENOENT) {
                LOG_DEBUG("ファイルが存在しません: " + path);
                return ResponseBuilder().status(kStatusNotFound).build();
            }
            LOG_DEBUG("stat失敗(その他エラー): " + path);
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        if (!S_ISREG(buf.st_mode)) {
            if (S_ISDIR(buf.st_mode)) {
                LOG_DEBUG("対象がディレクトリです: " + path);
            } else {
                LOG_DEBUG("対象が通常ファイルではありません: mode=other path=" + path);
            }
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        int remove_result = remove(path.c_str());
        LOG_DEBUG("remove結果: " + std::string(remove_result == 0 ? "成功" : "失敗") +
                  " errno=" + std::string(strerror(errno)));
        if (remove_result) {
            LOG_DEBUG("ファイル削除失敗: " + path);
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        LOG_DEBUG("ファイル削除成功: " + path);
        return ResponseBuilder().status(kStatusNoContent).build();
    }
} //namespace http
