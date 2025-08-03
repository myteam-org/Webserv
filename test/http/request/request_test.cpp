#include "http/request/request.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "config/context/documentRootConfig.hpp"
#include "config/context/locationContext.hpp"
#include "config/context/serverContext.hpp"
#include "http/method.hpp"  // kMethodGet / kMethodPost
#include "raw_headers.hpp"
#include "utils/types/option.hpp"

// 便利関数: vector<char> → string
static std::string bodyToString(const http::Request& req) {
    const std::vector<char>& b = req.getBody();
    return std::string(b.begin(), b.end());
}

// RawHeaders（キーは小文字前提）
static RawHeaders makeHeaders(
    std::initializer_list<std::pair<std::string, std::string>> kvs) {
    RawHeaders h;
    for (const auto& kv : kvs) h.insert(std::make_pair(kv.first, kv.second));
    return h;
}

// 先頭付近のヘルパ（C++98）
static std::vector<char> toVec(const std::string& s) {
    return std::vector<char>(s.begin(), s.end());
}

TEST(RequestTest, ConstructorAndGetters_FullArgs) {
    ::ServerContext server("server");
    ::LocationContext location("location");

    const http::HttpMethod method = http::kMethodGet;
    const std::string requestTarget = "/index.html?x=1";
    const std::string pathOnly = "/index.html";
    const std::string queryString = "x=1";
    const std::string version = "HTTP/1.1";

    // C++98: initializer_list は使わずに手で詰める
    RawHeaders headers;
    headers.insert(std::make_pair("host", "example.com"));
    headers.insert(std::make_pair("content-length", "12"));

    // C++98: toVec は (begin,end) で作る or 事前にヘルパーを用意
    const std::string bodyStr = "body content";
    std::vector<char> body(bodyStr.begin(), bodyStr.end());

    http::Request req(method, requestTarget, pathOnly, queryString, headers,
                      body, &server, &location);

    EXPECT_EQ(req.getMethod(), method);
    EXPECT_EQ(req.getRequestTarget(), requestTarget);
    EXPECT_EQ(req.getPath(), pathOnly);
    EXPECT_EQ(req.getQueryString(), queryString);
    EXPECT_EQ(req.getHttpVersion(), version);
    EXPECT_EQ(std::string(req.getBody().begin(), req.getBody().end()),
              "body content");

    // C++98: auto は使わない
    types::Option<std::string> h = req.getHeader("host");
    ASSERT_TRUE(h.isSome());
    EXPECT_EQ(h.unwrap(), "example.com");
}

static http::Request mkReq(::ServerContext& server,
                           ::LocationContext& location,
                           http::HttpMethod method,
                           const std::string& target,
                           const std::string& path,
                           const std::string& query,
                           const std::string& bodyStr,
                           const RawHeaders& headers /* = RawHeaders() */) {
    return http::Request(method, target, path, query,
                         headers, toVec(bodyStr), &server, &location);
}

TEST(RequestTest, EqualityOperator) {
    ::ServerContext server("server");
    ::LocationContext location("location");
    RawHeaders empty; // 空ヘッダ

    // 同値
    http::Request req1 = mkReq(server, location, http::kMethodGet, "/test", "/test", "", "", empty);
    http::Request req2 = mkReq(server, location, http::kMethodGet, "/test", "/test", "", "", empty);

    // 差分（method / target / body）
    http::Request req3 = mkReq(server, location, http::kMethodPost, "/test",  "/test",  "", "",    empty);
    http::Request req4 = mkReq(server, location, http::kMethodGet,  "/other", "/other", "", "",    empty);
    http::Request req6 = mkReq(server, location, http::kMethodGet,  "/test",  "/test",  "", "body", empty);

    EXPECT_TRUE (req1 == req2);
    EXPECT_FALSE(req1 == req3);
    EXPECT_FALSE(req1 == req4);
    // HTTP/1.1 固定のため version での差は検証しない
    EXPECT_FALSE(req1 == req6);
}

TEST(RequestTest, GetHeader_NoneByDefault) {
    ServerContext server("server");
    LocationContext location("location");

    http::Request req(http::kMethodGet, "/", "/", "", RawHeaders{},
                      std::vector<char>{}, &server, &location);

    auto h = req.getHeader("host");
    EXPECT_TRUE(h.isNone());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
