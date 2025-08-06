#include "upload.hpp"
#include "http/response/builder.hpp"
#include "utils/string.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

namespace http {

UploadFileHandler::UploadFileHandler(const DocumentRootConfig &documentRootConfig)
    : documentRootConfig_(documentRootConfig) {
}

Either<IAction *, Response> UploadFileHandler::serve(const Request &request) {
    return Right(this->serveInternal(request));
}

Response UploadFileHandler::serveInternal(const Request &request) const {
    // POST以外のメソッドは405 Method Not Allowedを返す
    if (request.getMethod() != kMethodPost) {
        return ResponseBuilder().status(kStatusMethodNotAllowed).build();
    }

    // リクエストボディが空の場合は400 Bad Requestを返す
    const std::vector<char> &body = request.getBody();
    if (body.empty()) {
        return ResponseBuilder().status(kStatusBadRequest).build();
    }

    // ファイルサイズの検証
    if (!isValidFileSize(body.size())) {
        return ResponseBuilder().status(kStatusPayloadTooLarge).build();
    }

    return saveUploadedFile(request);
}

Response UploadFileHandler::saveUploadedFile(const Request &request) const {
    const std::vector<char> &body = request.getBody();
    
    // Content-Typeヘッダからファイル名を取得（簡易実装）
    const types::Option<std::string> contentTypeHeader = 
        const_cast<Request&>(request).getHeader("Content-Type");
    
    std::string filename = determineFilename(contentTypeHeader);
    
    // 一意なファイル名を生成
    const std::string uniqueFilename = generateUniqueFilename(filename);

    // 保存先パスを構築
    const std::string uploadPath = documentRootConfig_.getRoot() + "/" + uniqueFilename;

    // ディレクトリの存在確認とファイル保存
    return saveFileToPath(body, uniqueFilename, uploadPath);
}

std::string UploadFileHandler::determineFilename(
    const types::Option<std::string> &contentTypeHeader) {
    const std::string filename = "uploaded_file";
    if (contentTypeHeader.isSome()) {
        const std::string contentType = contentTypeHeader.unwrap();
        // multipart/form-dataの場合の簡易的なファイル名抽出
        // 実際の実装では適切なmultipartパーサーが必要
        if (contentType.find("multipart/form-data") != std::string::npos) {
            return "form_upload";
        }
    }
    return filename;
}

Response UploadFileHandler::saveFileToPath(const std::vector<char> &body,
                                          const std::string &uniqueFilename,
                                          const std::string &uploadPath) const {
    // ディレクトリの存在確認
    struct stat st;
    if (stat(documentRootConfig_.getRoot().c_str(), &st) != 0) {
        return ResponseBuilder().status(kStatusInternalServerError).build();
    }

    // ファイルを保存
    std::ofstream file(uploadPath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return ResponseBuilder().status(kStatusInternalServerError).build();
    }

    file.write(&body[0], static_cast<std::streamsize>(body.size()));
    file.close();

    if (file.fail()) {
        return ResponseBuilder().status(kStatusInternalServerError).build();
    }

    // 成功レスポンスを構築
    return buildSuccessResponse(uniqueFilename, body.size());
}

Response UploadFileHandler::buildSuccessResponse(const std::string &filename,
                                                size_t fileSize) {
    std::stringstream responseBody;
    responseBody << "{\"status\":\"success\",\"filename\":\"" << filename 
                 << "\",\"size\":" << fileSize << "}";

    return ResponseBuilder()
        .text(responseBody.str(), kStatusCreated)
        .header("Content-Type", "application/json")
        .build();
}

std::string UploadFileHandler::generateUniqueFilename(const std::string &originalName) {
    // 現在時刻をベースにした一意なファイル名を生成
    const time_t now = time(0);
    std::stringstream ss;
    ss << now << "_" << originalName;
    return ss.str();
}

bool UploadFileHandler::isValidFileType(const std::string &filename) {
    (void)filename; // パラメータ未使用の警告を抑制
    // 基本的に全てのファイルタイプを許可
    // 実際の実装では設定に基づいて制限することができる
    return true;
}

bool UploadFileHandler::isValidFileSize(size_t size) {
    // デフォルトの最大サイズ: 10MB
    const size_t maxSize = static_cast<size_t>(10 * 1024 * 1024);
    return size <= maxSize;
}

} // namespace http
