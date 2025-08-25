#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>      // std::remove
#include <sys/stat.h>  // mkdir
#include <unistd.h>    // access

#include "http/handler/file/upload.hpp"
#include "http/request/request.hpp"
#include "config/context/documentRootConfig.hpp"
#include "utils/string.hpp"

namespace http {

class UploadFileHandlerTest : public ::testing::Test {
protected:
    std::string tempDir;
    std::string uploadPath;
    std::string filePath;

    void SetUp() override {
        tempDir = "tmp_upload_test";
        uploadPath = "/upload_test_file.txt";  // 先頭 '/' を含む通常の HTTP path
        filePath = utils::joinPath(tempDir, "upload_test_file.txt");

        mkdir(tempDir.c_str(), 0755);  // テスト用ディレクトリ作成
    }

    void TearDown() override {
        std::remove(filePath.c_str());  // ファイル削除
        rmdir(tempDir.c_str());         // ディレクトリ削除
    }

    static Request makeRequestPost(const std::string& requestTarget,
                                const std::vector<char>& body) {
        RawHeaders headers;
        const ServerContext*   server   = NULL;
        const LocationContext* location = NULL;

        // テストでは既に安全な文字列を使っているので、
        // pathOnly は requestTarget と同じで OK、queryString は空で OK。
        return Request(
            kMethodPost,
            requestTarget,            // requestTarget (raw)
            requestTarget,            // pathOnly (正規化済み相当として渡す)
            "",                       // queryString
            headers,
            body,
            server,
            location
        );
    }
};

TEST_F(UploadFileHandlerTest, UploadsFileSuccessfully) {
    DocumentRootConfig config;
    config.setRoot(tempDir);
    config.setEnabelUpload(ON);

    UploadFileHandler handler(config);

    RawHeaders headers;
    std::string content = "Hello, Upload!";
    std::vector<char> body(content.begin(), content.end());

    Request request = makeRequestPost(uploadPath, body);

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusCreated);

    std::ifstream ifs(filePath.c_str(), std::ios::binary);
    ASSERT_TRUE(ifs.is_open());

    std::stringstream ss;
    ss << ifs.rdbuf();
    EXPECT_EQ(ss.str(), content);
}

TEST_F(UploadFileHandlerTest, Returns403IfFileCannotBeOpened) {
    std::string unwritableDir = "/root";
    if (access(unwritableDir.c_str(), W_OK) == 0) {
        GTEST_SKIP() << "Running as root or /root is writable, skipping.";
    }

    DocumentRootConfig config;
    config.setRoot(unwritableDir);
    UploadFileHandler handler(config);

    RawHeaders headers;
    std::vector<char> body(10, 'x');

    Request request = makeRequestPost("/test.txt", body);

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusForbidden);
}

TEST_F(UploadFileHandlerTest, Returns404IfDirectoryDoesNotExist) {
    DocumentRootConfig config;
    config.setRoot("nonexistent_dir");
    config.setEnabelUpload(ON);
    UploadFileHandler handler(config);

    RawHeaders headers;
    std::vector<char> body(10, 'x');

    Request request = makeRequestPost("/missing_dir/file.txt", body);

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusNotFound);
}

}  // namespace http
