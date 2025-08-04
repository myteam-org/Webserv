#include <gtest/gtest.h>
#include "http/response/builder.hpp"
#include "http/status.hpp"
#include <fstream>

namespace http {

class ResponseBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a dummy file for testing file() method
        std::ofstream ofs("/tmp/test_file.txt");
        ofs << "file content";
        ofs.close();
    }

    void TearDown() override {
        std::remove("/tmp/test_file.txt");
    }
};

TEST_F(ResponseBuilderTest, BuildDefaultResponse) {
    ResponseBuilder builder;
    Response response = builder.build();
    EXPECT_EQ(response.getStatusCode(), kStatusOk);
    EXPECT_TRUE(response.getBody().isNone());
}

TEST_F(ResponseBuilderTest, SetStatus) {
    ResponseBuilder builder;
    Response response = builder.status(kStatusNotFound).build();
    EXPECT_EQ(response.getStatusCode(), kStatusNotFound);
}

TEST_F(ResponseBuilderTest, SetTextBody) {
    ResponseBuilder builder;
    Response response = builder.text("Hello, World!").build();
    EXPECT_EQ(response.getStatusCode(), kStatusOk);
    EXPECT_TRUE(response.getBody().isSome());
    EXPECT_EQ(response.getBody().unwrap(), "Hello, World!");
    // TODO: ヘッダーのテストを追加
}

TEST_F(ResponseBuilderTest, SetHtmlBody) {
    ResponseBuilder builder;
    Response response = builder.html("<h1>Hello</h1>").build();
    EXPECT_EQ(response.getStatusCode(), kStatusOk);
    EXPECT_TRUE(response.getBody().isSome());
    EXPECT_EQ(response.getBody().unwrap(), "<h1>Hello</h1>");
    // TODO: ヘッダーのテストを追加
}

TEST_F(ResponseBuilderTest, SetRedirect) {
    ResponseBuilder builder;
    Response response = builder.redirect("/new-path").build();
    EXPECT_EQ(response.getStatusCode(), kStatusFound);
    // TODO: ヘッダーのテストを追加
}

TEST_F(ResponseBuilderTest, SetFile) {
    ResponseBuilder builder;
    Response response = builder.file("/tmp/test_file.txt").build();
    EXPECT_EQ(response.getStatusCode(), kStatusOk);
    EXPECT_TRUE(response.getBody().isSome());
    EXPECT_EQ(response.getBody().unwrap(), "file content");
    // TODO: ヘッダーのテストを追加
}

TEST_F(ResponseBuilderTest, SetFileNonExisting) {
    ResponseBuilder builder;
    Response response = builder.file("/tmp/non_existing_file.txt").build();
    EXPECT_EQ(response.getStatusCode(), kStatusNotFound);
    EXPECT_TRUE(response.getBody().isNone());
}

} // namespace http
