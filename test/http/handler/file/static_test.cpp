#include <gtest/gtest.h>
#include "http/handler/file/static.hpp"
#include "http/request/request.hpp"
#include "config/context/documentRootConfig.hpp"
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>

namespace http {

class StaticFileHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a dummy root directory and some files/directories for testing
        system("mkdir -p /tmp/www/dir1");
        system("echo 'hello world' > /tmp/www/index.html");
        system("echo 'file content' > /tmp/www/file.txt");
        system("echo 'dir1 content' > /tmp/www/dir1/file_in_dir1.txt");
        docRootConfig_.setRoot("/tmp/www");
        docRootConfig_.setIndex("index.html");
        docRootConfig_.setAutoIndex(OFF);
        
        // テスト用の共通オブジェクトを初期化
        server_ = NULL;
        location_ = NULL;
    }
    
    void TearDown() override {
        system("rm -rf /tmp/www");
    }
    
    // Requestオブジェクトを作成するヘルパーメソッド
    Request createRequest(const std::string& target) {
        RawHeaders headers;
        std::vector<char> body;

        // request-target を path と query に分割
        std::string pathOnly = target;
        std::string queryString;
        const std::size_t qm = target.find('?');
        if (qm != std::string::npos) {
            pathOnly = target.substr(0, qm);
            queryString = target.substr(qm + 1);
        }
        // テスト入力は既に正規化済み想定（"/", "/dir1", "/file.txt" など）
        return Request(
            kMethodGet,    // method
            target,        // requestTarget (raw)
            pathOnly,      // pathOnly (normalized想定)
            queryString,   // queryString
            headers,       // headers
            body,          // body
            server_,       // server
            location_      // location
        );
    }
    
    DocumentRootConfig docRootConfig_;
    const ServerContext* server_;
    const LocationContext* location_;
};

TEST_F(StaticFileHandlerTest, ServeExistingFile) {
    StaticFileHandler handler(docRootConfig_);
    Request request = createRequest("/file.txt");
    Either<IAction *, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusOk);
    EXPECT_EQ(result.unwrapRight().getBody().unwrap(), "file content\n");
}

TEST_F(StaticFileHandlerTest, ServeNonExistingFile) {
    StaticFileHandler handler(docRootConfig_);
    Request request = createRequest("/non_existing.txt");
    Either<IAction *, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusNotFound);
}

TEST_F(StaticFileHandlerTest, ServeDirectoryWithIndexFile) {
    StaticFileHandler handler(docRootConfig_);
    Request request = createRequest("/");
    Either<IAction *, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusOk);
    EXPECT_EQ(result.unwrapRight().getBody().unwrap(), "hello world\n");
}

TEST_F(StaticFileHandlerTest, ServeDirectoryWithoutTrailingSlash) {
    StaticFileHandler handler(docRootConfig_);
    Request request = createRequest("/dir1");
    Either<IAction *, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusFound);
}

TEST_F(StaticFileHandlerTest, ServeDirectoryWithAutoindexEnabled) {
    docRootConfig_.setAutoIndex(ON);
    StaticFileHandler handler(docRootConfig_);
    Request request = createRequest("/dir1/");
    Either<IAction *, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusOk);
    EXPECT_TRUE(result.unwrapRight().getBody().unwrap().find("file_in_dir1.txt") != std::string::npos);
}

TEST_F(StaticFileHandlerTest, ServeDirectoryWithAutoindexDisabledAndNoIndexFile) {
    StaticFileHandler handler(docRootConfig_); // Autoindex is OFF by default
    Request request = createRequest("/dir1/");
    Either<IAction *, Response> result = handler.serve(request);
    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusForbidden);
}

} // namespace http
