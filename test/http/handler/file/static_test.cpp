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
        return Request(kMethodGet, target, headers, body, server_, location_);
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
