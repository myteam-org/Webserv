#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>  // std::remove

#include "http/handler/file/upload.hpp"
#include "http/request/request.hpp"
#include "config/context/documentRootConfig.hpp"

namespace http {

class UploadFileHandlerTest : public ::testing::Test {
protected:
    std::string tempDir;
    std::string filePath;
    std::string uploadPath;

    void SetUp() {
        tempDir = "./tmp_upload_test";
        uploadPath = "/upload_test_file.txt";
        filePath = tempDir + uploadPath;

        // 一時ディレクトリを作成（存在しない場合）
        mkdir(tempDir.c_str(), 0755);
    }

    void TearDown() {
        std::remove(filePath.c_str());  // ファイル削除
        rmdir(tempDir.c_str());         // ディレクトリ削除
    }
};

TEST_F(UploadFileHandlerTest, UploadsFileSuccessfully) {
    // ハンドラの準備
    DocumentRootConfig config;
    config.setRoot(tempDir);  // temp upload dir

    UploadFileHandler handler(config);

    // ダミーリクエスト作成
    RawHeaders headers;
    std::string testContent = "Hello, Upload!";
    std::vector<char> body(testContent.begin(), testContent.end());

    const ServerContext* server = NULL;
    const LocationContext* location = NULL;

    Request request(
        kMethodPost,           // POST メソッド
        uploadPath,            // pathOnly 相当
        headers,
        body,
        server,
        location
    );

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusCreated);

    // 実際にファイルができたか確認
    std::ifstream ifs(filePath.c_str(), std::ios::binary);
    ASSERT_TRUE(ifs.is_open());

    std::stringstream ss;
    ss << ifs.rdbuf();
    EXPECT_EQ(ss.str(), testContent);
}

}  // namespace http
