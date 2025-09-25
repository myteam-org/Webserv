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
        uploadPath = "/upload";  // アップロードエンドポイント
        filePath = tempDir + "/test_file.txt";  // 実際に作成されるファイルパス

        mkdir(tempDir.c_str(), 0755);  // テスト用ディレクトリ作成
    }

    void TearDown() override {
        std::remove(filePath.c_str());  // ファイル削除
        rmdir(tempDir.c_str());         // ディレクトリ削除
    }

    static Request makeMultipartRequest(const std::string& requestTarget,
                                      const std::string& filename,
                                      const std::string& fileContent) {
        const std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
        const std::string contentType = "multipart/form-data; boundary=" + boundary;
        
        // multipart/form-data形式のボディを構築
        std::string bodyStr;
        bodyStr += "--" + boundary + "\r\n";
        bodyStr += "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n";
        bodyStr += "Content-Type: text/plain\r\n";
        bodyStr += "\r\n";
        bodyStr += fileContent;
        bodyStr += "\r\n--" + boundary + "--\r\n";
        
        std::vector<char> body(bodyStr.begin(), bodyStr.end());

        RawHeaders headers;
        headers["content-type"] = contentType;  // Content-Typeヘッダーを設定

        const ServerContext* server = NULL;
        const LocationContext* location = NULL;

        return Request(
            kMethodPost,
            requestTarget,            // requestTarget (raw)
            requestTarget,            // pathOnly
            "",                       // queryString
            headers,
            body,
            server,
            location
        );
    }

    static Request makeInvalidRequest(const std::string& requestTarget,
                                    const std::vector<char>& body) {
        RawHeaders headers;
        // Content-Typeヘッダーを意図的に設定しない、または無効な値にする
        headers["content-type"] = "text/plain";  // multipart/form-dataではない

        const ServerContext* server = NULL;
        const LocationContext* location = NULL;

        return Request(
            kMethodPost,
            requestTarget,
            requestTarget,
            "",
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
    config.setEnableUpload(ON);

    UploadFileHandler handler(config);

    std::string content = "Hello, Upload!";
    std::string filename = "test_file.txt";

    Request request = makeMultipartRequest(uploadPath, filename, content);

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusCreated);

    // ファイルが正しく作成されているかチェック
    std::ifstream ifs(filePath.c_str(), std::ios::binary);
    ASSERT_TRUE(ifs.is_open()) << "Upload file was not created: " << filePath;

    std::stringstream ss;
    ss << ifs.rdbuf();
    EXPECT_EQ(ss.str(), content);
}

TEST_F(UploadFileHandlerTest, Returns400IfContentTypeIsInvalid) {
    DocumentRootConfig config;
    config.setRoot(tempDir);
    config.setEnableUpload(ON);

    UploadFileHandler handler(config);

    std::string content = "Hello, Upload!";
    std::vector<char> body(content.begin(), content.end());

    Request request = makeInvalidRequest(uploadPath, body);

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusBadRequest);
}

TEST_F(UploadFileHandlerTest, Returns403IfUploadIsDisabled) {
    DocumentRootConfig config;
    config.setRoot(tempDir);
    config.setEnableUpload(OFF);  // アップロード無効

    UploadFileHandler handler(config);

    std::string content = "Hello, Upload!";
    std::string filename = "test_file.txt";

    Request request = makeMultipartRequest(uploadPath, filename, content);

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusForbidden);
}

TEST_F(UploadFileHandlerTest, Returns403IfFileCannotBeOpened) {
    std::string unwritableDir = "/root";
    if (access(unwritableDir.c_str(), W_OK) == 0) {
        GTEST_SKIP() << "Running as root or /root is writable, skipping.";
    }

    DocumentRootConfig config;
    config.setRoot(unwritableDir);
    config.setEnableUpload(ON);
    
    UploadFileHandler handler(config);

    std::string content = "test content";
    std::string filename = "test.txt";

    Request request = makeMultipartRequest(uploadPath, filename, content);

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusForbidden);
}

TEST_F(UploadFileHandlerTest, Returns404IfDirectoryDoesNotExist) {
    DocumentRootConfig config;
    config.setRoot("nonexistent_dir");  // 存在しないディレクトリ
    config.setEnableUpload(ON);
    
    UploadFileHandler handler(config);

    std::string content = "test content";
    std::string filename = "file.txt";

    Request request = makeMultipartRequest(uploadPath, filename, content);

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusNotFound);
}

TEST_F(UploadFileHandlerTest, Returns400IfNoBoundaryInContentType) {
    DocumentRootConfig config;
    config.setRoot(tempDir);
    config.setEnableUpload(ON);

    UploadFileHandler handler(config);

    RawHeaders headers;
    headers["content-type"] = "multipart/form-data";  // boundaryがない

    std::string content = "test content";
    std::vector<char> body(content.begin(), content.end());

    const ServerContext* server = NULL;
    const LocationContext* location = NULL;

    Request request(
        kMethodPost,
        uploadPath,
        uploadPath,
        "",
        headers,
        body,
        server,
        location
    );

    Either<IAction*, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusBadRequest);
}

}  // namespace http
