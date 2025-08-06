#include <gtest/gtest.h>
#include "http/handler/file/upload.hpp"
#include "http/request/request.hpp"
#include "config/context/documentRootConfig.hpp"
#include "http/request/read/raw_headers.hpp"
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>

namespace http {

class UploadFileHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テスト用のアップロードディレクトリを作成
        system("mkdir -p /tmp/upload_test");
        docRootConfig_.setRoot("/tmp/upload_test");
        docRootConfig_.setIndex("index.html");
        docRootConfig_.setAutoIndex(OFF);
        
        // テスト用の共通オブジェクトを初期化
        server_ = NULL;
        location_ = NULL;
    }
    
    void TearDown() override {
        // テスト用ディレクトリを削除
        system("rm -rf /tmp/upload_test");
    }

    DocumentRootConfig docRootConfig_;
    const ServerContext* server_;
    const LocationContext* location_;
};

TEST_F(UploadFileHandlerTest, HandlePostRequestWithFileData) {
    UploadFileHandler handler(docRootConfig_);
    
    // テスト用のファイルデータを作成
    std::string testData = "Hello, World! This is test file content.";
    std::vector<char> body(testData.begin(), testData.end());
    
    // テスト用のヘッダー
    RawHeaders headers;
    headers["Content-Type"] = "application/octet-stream";
    headers["Content-Length"] = "41";
    
    // POSTリクエストを作成
    Request request(kMethodPost, "/upload", headers, body, server_, location_);
    
    // ハンドラーを実行
    Either<IAction *, Response> result = handler.serve(request);
    
    // レスポンスが成功であることを確認
    ASSERT_TRUE(result.isRight());
    Response response = result.unwrapRight();
    
    // ステータスコードが201 Createdであることを確認
    EXPECT_EQ(response.getStatusCode(), kStatusCreated);
    
    // ファイルが実際に保存されていることを確認
    // (実装によってファイル名が変わるため、ディレクトリ内のファイル数をチェック)
    DIR* dir = opendir("/tmp/upload_test");
    ASSERT_NE(dir, nullptr);
    
    int fileCount = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // 通常ファイルのみカウント
            fileCount++;
        }
    }
    closedir(dir);
    
    EXPECT_EQ(fileCount, 1);
}

TEST_F(UploadFileHandlerTest, RejectNonPostMethods) {
    UploadFileHandler handler(docRootConfig_);
    
    std::vector<char> body;
    RawHeaders headers;
    
    // GETリクエストを作成
    Request request(kMethodGet, "/upload", headers, body, server_, location_);
    
    // ハンドラーを実行
    Either<IAction *, Response> result = handler.serve(request);
    
    // レスポンスが成功であることを確認
    ASSERT_TRUE(result.isRight());
    Response response = result.unwrapRight();
    
    // ステータスコードが405 Method Not Allowedであることを確認
    EXPECT_EQ(response.getStatusCode(), kStatusMethodNotAllowed);
}

TEST_F(UploadFileHandlerTest, RejectEmptyBody) {
    UploadFileHandler handler(docRootConfig_);
    
    std::vector<char> emptyBody;
    RawHeaders headers;
    
    // 空のボディでPOSTリクエストを作成
    Request request(kMethodPost, "/upload", headers, emptyBody, server_, location_);
    
    // ハンドラーを実行
    Either<IAction *, Response> result = handler.serve(request);
    
    // レスポンスが成功であることを確認
    ASSERT_TRUE(result.isRight());
    Response response = result.unwrapRight();
    
    // ステータスコードが400 Bad Requestであることを確認
    EXPECT_EQ(response.getStatusCode(), kStatusBadRequest);
}

} // namespace http