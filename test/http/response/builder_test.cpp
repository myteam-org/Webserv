#include <gtest/gtest.h>
#include "http/response/builder.hpp"
#include "http/response/response_header_types.hpp"
#include "http/status.hpp"
#include <fstream>
#include <cstdio>

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
    const http::ResponseHeaderFields& h = response.getHeaders();
    EXPECT_EQ(response.getStatusCode(), kStatusOk);
    EXPECT_TRUE(response.getBody().isNone());
    ASSERT_TRUE(h.find("content-length") != h.end());
    EXPECT_EQ(h.find("content-length")->second, "0");
    ASSERT_TRUE(h.find("connection") != h.end());
    EXPECT_EQ(h.find("connection")->second, "keep-alive");
}

TEST_F(ResponseBuilderTest, SetStatus) {
    ResponseBuilder builder;
    Response response = builder.status(kStatusNotFound).build();
    EXPECT_EQ(response.getStatusCode(), kStatusNotFound);
}

TEST_F(ResponseBuilderTest, SetHtmlBody) {
    ResponseBuilder builder;
    Response response = builder.html("<h1>Hello</h1>").build();
    const http::ResponseHeaderFields& h = response.getHeaders();
    EXPECT_EQ(response.getStatusCode(), kStatusOk);
    EXPECT_TRUE(response.getBody().isSome());
    EXPECT_EQ(response.getBody().unwrap(), "<h1>Hello</h1>");
    ASSERT_TRUE(h.find("content-type") != h.end());
    EXPECT_EQ(h.find("content-type")->second, "text/html; charset=UTF-8"); // ← 修正
    ASSERT_TRUE(h.find("content-length") != h.end());
    EXPECT_EQ(h.find("content-length")->second, "14"); // ← 14 に修正
}

TEST_F(ResponseBuilderTest, SetRedirect) {
    ResponseBuilder builder;
    Response response = builder.redirect("/new-path").build();
    EXPECT_EQ(response.getStatusCode(), kStatusFound);
    EXPECT_TRUE(response.getBody().isNone());
    const http::ResponseHeaderFields& h = response.getHeaders();
    ASSERT_TRUE(h.find("location") != h.end());
    EXPECT_EQ(h.find("location")->second, "/new-path");
    ASSERT_TRUE(h.find("content-length") != h.end());
    EXPECT_EQ(h.find("content-length")->second, "0");
}


TEST_F(ResponseBuilderTest, SetFile) {
    ResponseBuilder builder;
    Response response = builder.file("/tmp/test_file.txt").build();
    EXPECT_EQ(response.getStatusCode(), kStatusOk);
    EXPECT_TRUE(response.getBody().isSome());
    EXPECT_EQ(response.getBody().unwrap(), "file content");
    const http::ResponseHeaderFields& h = response.getHeaders();
    ASSERT_TRUE(h.find("content-type") != h.end());
    EXPECT_TRUE(h.find("content-type")->second.find("text/") == 0); // mime 実装に依存するならゆるく
    ASSERT_TRUE(h.find("content-length") != h.end());
    EXPECT_EQ(h.find("content-length")->second, "12"); // "file content" は 12
}

TEST_F(ResponseBuilderTest, SetFileNonExisting) {
    ResponseBuilder builder;
    Response response = builder.file("/tmp/non_existing_file.txt").build();
    EXPECT_EQ(response.getStatusCode(), kStatusNotFound);
    EXPECT_TRUE(response.getBody().isNone());
    const http::ResponseHeaderFields& h = response.getHeaders();
    ASSERT_TRUE(h.find("content-length") != h.end());
    EXPECT_EQ(h.find("content-length")->second, "0");
}

TEST_F(ResponseBuilderTest, TransferEncodingExcludesContentLength) {
    ResponseBuilder builder;
    Response response = builder
        .header("Transfer-Encoding", "chunked")
        .text("ABCDE")  // 一旦 CL が付くが、build() で削除される想定
        .build();

    const http::ResponseHeaderFields& h = response.getHeaders();
    ASSERT_TRUE(h.find("transfer-encoding") != h.end());
    EXPECT_EQ(h.find("transfer-encoding")->second, "chunked");
    EXPECT_TRUE(h.find("content-length") == h.end()); // 併用禁止
}

TEST_F(ResponseBuilderTest, NoBodyStatus205) {
    Response response = ResponseBuilder().text("x").status(kStatusResetContent).build();
    const http::ResponseHeaderFields& h = response.getHeaders();
    ASSERT_TRUE(h.find("content-length") != h.end());
    EXPECT_EQ(h.find("content-length")->second, "0");
}

} // namespace http
